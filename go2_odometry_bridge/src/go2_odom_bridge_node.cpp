#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <memory>
#include <cmath>

#include "unitree_go/msg/sport_mode_state.hpp"

class Go2OdomBridgeNode : public rclcpp::Node {
public:
    Go2OdomBridgeNode() : Node("go2_odom_bridge_node") {
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("/odom", 50);
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        state_sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
            "/lf/sportmodestate", 10,
            std::bind(&Go2OdomBridgeNode::state_callback, this, std::placeholders::_1));

        x_ = 0.0; y_ = 0.0; yaw_ = 0.0;
        last_time_ = this->now();

        //CHANGES-HERE
        // --- BIAS ESTIMATION ---
        bias_window_.fill(0.0);
        bias_index_ = 0;
        bias_sum_ = 0.0;
        bias_ = 0.0;

        RCLCPP_INFO(this->get_logger(), "Go2 Odometry Bridge: Publishing /odom + TF (odom→base_link)");
    }

private:
    void state_callback(const unitree_go::msg::SportModeState::SharedPtr msg) {
auto current_time = this->now();
        double dt = (current_time - last_time_).seconds();
        if (dt <= 0.0 || dt > 0.5) return;  // Prevent jumps

        double vx = msg->velocity[0];
        double vy = msg->velocity[1];
        double wz_raw = msg->yaw_speed;

        // --- STEP 1: DEADZONE (for noise) ---
        const double DEADZONE = 0.01;  // rad/s (~0.57 deg/s)
        double wz_clean = (std::abs(wz_raw) < DEADZONE) ? 0.0 : wz_raw;

        // --- STEP 2: UPDATE BIAS (only when robot is still) ---
        bool robot_still = (std::abs(vx) < 0.05 && std::abs(vy) < 0.05);
        if (robot_still && std::abs(wz_clean) < 0.05) {
            // Update rolling window
            bias_sum_ -= bias_window_[bias_index_];
            bias_window_[bias_index_] = wz_raw;
            bias_sum_ += wz_raw;
            bias_index_ = (bias_index_ + 1) % BIAS_WINDOW_SIZE;
            bias_ = bias_sum_ / BIAS_WINDOW_SIZE;
        }

        // --- STEP 3: APPLY BIAS CORRECTION ---
        double wz = wz_clean - bias_;

        // --- DEAD RECKONING ---
        double delta_x = (vx * std::cos(yaw_) - vy * std::sin(yaw_)) * dt;
        double delta_y = (vx * std::sin(yaw_) + vy * std::cos(yaw_)) * dt;
        double delta_yaw = wz * dt;

        x_ += delta_x;
        y_ += delta_y;
        yaw_ += delta_yaw;


        // --- PUBLISH ODOM ---
        auto odom_msg = nav_msgs::msg::Odometry();
        odom_msg.header.stamp = current_time;
        odom_msg.header.frame_id = "odom";
        odom_msg.child_frame_id = "base_link";
        odom_msg.pose.pose.position.x = x_;
        odom_msg.pose.pose.position.y = y_;
        odom_msg.pose.pose.position.z = 0.0;
        tf2::Quaternion q; q.setRPY(0, 0, yaw_);
        odom_msg.pose.pose.orientation = tf2::toMsg(q);
        odom_msg.twist.twist.linear.x = vx;
        odom_msg.twist.twist.linear.y = vy;
        odom_msg.twist.twist.angular.z = wz;
        odom_pub_->publish(odom_msg);

        // --- PUBLISH TF ---
        geometry_msgs::msg::TransformStamped tf;
        tf.header.stamp = current_time;
        tf.header.frame_id = "odom";
        tf.child_frame_id = "base_link";
        tf.transform.translation.x = x_;
        tf.transform.translation.y = y_;
        tf.transform.translation.z = 0.0;
        tf.transform.rotation = odom_msg.pose.pose.orientation;
        tf_broadcaster_->sendTransform(tf);

        last_time_ = current_time;

        // --- LOG BIAS (optional) ---
        if (bias_index_ == 0) {
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                                 "Yaw bias: %.6f rad/s", bias_);
        }
    }

    // --- BIAS FILTER ---
    static constexpr int BIAS_WINDOW_SIZE = 100;
    std::array<double, BIAS_WINDOW_SIZE> bias_window_;
    int bias_index_ = 0;
    double bias_sum_ = 0.0;
    double bias_ = 0.0;

    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr state_sub_;

    double x_, y_, yaw_;
    rclcpp::Time last_time_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Go2OdomBridgeNode>());
    rclcpp::shutdown();
    return 0;
}