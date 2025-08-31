#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <memory>
#include <cmath>
#include <fstream>      // Required for file I/O
#include <deque>        // Required for the sliding window history
#include <vector>       // Required for sorting
#include <algorithm>    // Required for std::sort

// Unitree SDK
#include "unitree/robot/channel/channel_subscriber.hpp"
#include "unitree/robot/channel/channel_factory.hpp"
#include "unitree/idl/go2/SportModeState_.hpp"

// Define the size of the median filter window. Must be an odd number.
// A larger number means more smoothing but also more delay. 5 is a good starting point.
static constexpr int MEDIAN_WINDOW_SIZE = 5;

class OdomBridgeNode : public rclcpp::Node {
public:
  OdomBridgeNode(const std::string &iface)
  : Node("odom_bridge_node") {
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/odom", 50);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // Initialize state [x, y, yaw] to zero.
    x_ = 0.0;
    y_ = 0.0;
    yaw_ = 0.0;

    last_time_ = now();

    // --- LOGGING SETUP ---
    input_log_file_.open("odom_inputs.txt", std::ios::out | std::ios::trunc);
    output_log_file_.open("odom_outputs.txt", std::ios::out | std::ios::trunc);
    if (input_log_file_.is_open() && output_log_file_.is_open()) {
        RCLCPP_INFO(this->get_logger(), "Logging odom data to odom_inputs.txt and odom_outputs.txt");
        input_log_file_ << "timestamp,raw_vx,raw_vy,raw_wz\n";
        output_log_file_ << "timestamp,dt,x,y,yaw,filtered_vx,filtered_vy,filtered_wz\n";
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to open log files for writing!");
    }

    unitree::robot::ChannelFactory::Instance()->Init(0, iface);
    subscriber_ = std::make_shared<
      unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>>(
        "rt/sportmodestate");
    subscriber_->InitChannel([this](auto msg) { this->stateCallback(msg); });

    RCLCPP_INFO(this->get_logger(), "Odom bridge node started with median filter enabled (window size: %d).", MEDIAN_WINDOW_SIZE);
  }

  ~OdomBridgeNode() {
      if (input_log_file_.is_open()) input_log_file_.close();
      if (output_log_file_.is_open()) output_log_file_.close();
  }

private:
  // --- START MEDIAN FILTER IMPLEMENTATION ---
  void updateHistory(std::deque<double>& history, double newValue) {
    history.push_back(newValue);
    if (history.size() > MEDIAN_WINDOW_SIZE) {
        history.pop_front();
    }
  }

  double calculateMedian(const std::deque<double>& history) {
    if (history.empty()) {
        return 0.0;
    }
    // Create a temporary vector to sort, leaving the original deque unmodified
    std::vector<double> sorted_history(history.begin(), history.end());
    std::sort(sorted_history.begin(), sorted_history.end());
    // Return the middle element
    return sorted_history[sorted_history.size() / 2];
  }
  // --- END MEDIAN FILTER IMPLEMENTATION ---

  void stateCallback(const void *m) {
    const auto *dds = static_cast<const unitree_go::msg::dds_::SportModeState_ *>(m);
    auto now = get_clock()->now();
    double dt = (now - last_time_).seconds();
    if (dt <= 0) return;

    double raw_vx = dds->velocity()[0];
    double raw_vy = dds->velocity()[1];
    double raw_wz = dds->yaw_speed();

    // --- LOG RAW INPUTS ---
    if (input_log_file_.is_open()) {
        input_log_file_ << now.seconds() << "," << raw_vx << "," << raw_vy << "," << raw_wz << "\n";
    }

    // --- APPLY MEDIAN FILTER ---
    updateHistory(vx_history_, raw_vx);
    updateHistory(vy_history_, raw_vy);
    updateHistory(wz_history_, raw_wz);

    double vx = calculateMedian(vx_history_);
    double vy = calculateMedian(vy_history_);
    double wz = calculateMedian(wz_history_);
    // --- FILTERING COMPLETE ---

    // --- Dead-Reckoning Integration using FILTERED velocities ---
    double current_yaw = yaw_;
    x_ += (vx * cos(current_yaw) - vy * sin(current_yaw)) * dt;
    y_ += (vx * sin(current_yaw) + vy * cos(current_yaw)) * dt;
    yaw_ += wz * dt;
    wrapYaw(yaw_);

    // --- LOG FILTERED OUTPUTS ---
    if (output_log_file_.is_open()) {
        output_log_file_ << now.seconds() << "," << dt << "," << x_ << "," << y_ << "," << yaw_ << "," << vx << "," << vy << "," << wz << "\n";
    }

    // --- Publish Odometry Message ---
    auto odom = std::make_unique<nav_msgs::msg::Odometry>();
    odom->header.stamp = now;
    odom->header.frame_id = "odom";
    odom->child_frame_id = "base_link";

    odom->pose.pose.position.x = x_;
    odom->pose.pose.position.y = y_;
    odom->pose.pose.position.z = 0.0;

    tf2::Quaternion q;
    q.setRPY(0, 0, yaw_);
    odom->pose.pose.orientation = tf2::toMsg(q);

    odom->twist.twist.linear.x = vx;
    odom->twist.twist.linear.y = vy;
    odom->twist.twist.angular.z = wz;

    odom_pub_->publish(std::move(odom));

    // --- Publish TF transform (odom -> base_link) ---
    geometry_msgs::msg::TransformStamped t;
    t.header.stamp = now;
    t.header.frame_id = "odom";
    t.child_frame_id = "base_link";
    t.transform.translation.x = x_;
    t.transform.translation.y = y_;
    t.transform.translation.z = 0.0;
    t.transform.rotation = tf2::toMsg(q);
    tf_broadcaster_->sendTransform(t);

    last_time_ = now;
  }

  void wrapYaw(double &a) {
    while (a > M_PI) a -= 2 * M_PI;
    while (a < -M_PI) a += 2 * M_PI;
  }

  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::shared_ptr<unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>> subscriber_;

  double x_, y_, yaw_;
  rclcpp::Time last_time_;

  std::ofstream input_log_file_;
  std::ofstream output_log_file_;

  std::deque<double> vx_history_;
  std::deque<double> vy_history_;
  std::deque<double> wz_history_;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <network_interface>\n";
    return 1;
  }
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OdomBridgeNode>(argv[1]);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
