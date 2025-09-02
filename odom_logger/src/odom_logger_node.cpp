#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <cmath>
#include <fstream>      // Required for file I/O
#include <deque>        // Required for the sliding window history
#include <vector>       // Required for sorting
#include <algorithm>    // Required for std::sort
#include <iomanip>      // Required for std::setprecision

// Unitree SDK
#include "unitree/robot/channel/channel_subscriber.hpp"
#include "unitree/robot/channel/channel_factory.hpp"
#include "unitree/idl/go2/SportModeState_.hpp"

static constexpr int MEDIAN_WINDOW_SIZE = 5;

class OdomLoggerNode : public rclcpp::Node {
public:
  OdomLoggerNode(const std::string &iface)
  : Node("odom_logger_node") {
    // Initialize state [x, y, yaw] to zero.
    x_ = 0.0;
    y_ = 0.0;
    yaw_ = 0.0;
    last_time_ = now();

    // --- LOGGING SETUP ---
    raw_log_file_.open("raw_odom_log.csv", std::ios::out | std::ios::trunc);
    processed_log_file_.open("processed_odom_log.csv", std::ios::out | std::ios::trunc);

    if (raw_log_file_.is_open() && processed_log_file_.is_open()) {
        RCLCPP_INFO(this->get_logger(), "Logging raw data to raw_odom_log.csv");
        RCLCPP_INFO(this->get_logger(), "Logging processed data to processed_odom_log.csv");
        // Write CSV headers
        raw_log_file_ << "timestamp,raw_vx,raw_vy,raw_wz\n";
        processed_log_file_ << "timestamp,dt,x,y,yaw,filtered_vx,filtered_vy,filtered_wz\n";
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to open one or more log files for writing!");
        rclcpp::shutdown();
    }

    // --- UNITREE SDK SETUP ---
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);
    subscriber_ = std::make_shared<
      unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>>(
        "rt/sportmodestate");
    subscriber_->InitChannel([this](auto msg) { this->stateCallback(msg); });

    RCLCPP_INFO(this->get_logger(), "Odometry logger node started.");
  }

  ~OdomLoggerNode() {
      if (raw_log_file_.is_open()) raw_log_file_.close();
      if (processed_log_file_.is_open()) processed_log_file_.close();
      RCLCPP_INFO(this->get_logger(), "Log files closed.");
  }

private:
  void updateHistory(std::deque<double>& history, double newValue) {
    history.push_back(newValue);
    if (history.size() > MEDIAN_WINDOW_SIZE) {
        history.pop_front();
    }
  }

  double calculateMedian(const std::deque<double>& history) {
    if (history.empty()) return 0.0;
    std::vector<double> sorted_history(history.begin(), history.end());
    std::sort(sorted_history.begin(), sorted_history.end());
    return sorted_history[sorted_history.size() / 2];
  }

  void stateCallback(const void *m) {
    const auto *dds = static_cast<const unitree_go::msg::dds_::SportModeState_ *>(m);
    auto now = get_clock()->now();
    double dt = (now - last_time_).seconds();
    if (dt <= 0.001) return; // Prevent division by zero or weird behavior on startup

    // --- 1. GET AND LOG RAW DATA ---
    double raw_vx = dds->velocity()[0];
    double raw_vy = dds->velocity()[1];
    double raw_wz = dds->yaw_speed();

    if (raw_log_file_.is_open()) {
        raw_log_file_ << std::fixed << std::setprecision(6) << now.seconds() << ","
                      << raw_vx << "," << raw_vy << "," << raw_wz << "\n";
    }

    // --- 2. PROCESS THE DATA (Filtering and Integration) ---
    updateHistory(vx_history_, raw_vx);
    updateHistory(vy_history_, raw_wz);
    updateHistory(wz_history_, raw_wz);

    double filtered_vx = calculateMedian(vx_history_);
    double filtered_vy = calculateMedian(vy_history_);
    double filtered_wz = calculateMedian(wz_history_);

    double current_yaw = yaw_;
    x_ += (filtered_vx * cos(current_yaw) - filtered_vy * sin(current_yaw)) * dt;
    y_ += (filtered_vx * sin(current_yaw) + filtered_vy * cos(current_yaw)) * dt;
    yaw_ += filtered_wz * dt;

    // --- 3. LOG THE PROCESSED DATA ---
    if (processed_log_file_.is_open()) {
        processed_log_file_ << std::fixed << std::setprecision(6) << now.seconds() << ","
                            << dt << "," << x_ << "," << y_ << "," << yaw_ << ","
                            << filtered_vx << "," << filtered_vy << "," << filtered_wz << "\n";
    }

    last_time_ = now;
  }

  // Unitree SDK subscriber
  std::shared_ptr<unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>> subscriber_;

  // State variables
  double x_, y_, yaw_;
  rclcpp::Time last_time_;

  // Logging file streams
  std::ofstream raw_log_file_;
  std::ofstream processed_log_file_;

  // Median filter history deques
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
  auto node = std::make_shared<OdomLoggerNode>(argv[1]);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
