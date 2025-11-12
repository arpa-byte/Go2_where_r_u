#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <memory>
#include <cmath>
#include <algorithm>

#include "unitree_go/msg/sport_mode_state.hpp"

class Go2OdomNavNode : public rclcpp::Node {
public:
    Go2OdomNavNode() : Node("go2_odom_nav_node") {
        // Publisher for /odom
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("/odom", 50);

        // TF broadcaster
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        // Subscriber to Go2 state
        state_sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
            "/lf/sportmodestate", 10,
            std::bind(&Go2OdomNavNode::state_callback, this, std::placeholders::_1));

        // Initialize pose
        x_ = 0.0;
        y_ = 0.0;
        yaw_ = 0.0;
        last_time_ = this->now();

        // Initialize filters
        wz_filtered_ = 0.0;
        vx_filtered_ = 0.0;
        vy_filtered_ = 0.0;

        RCLCPP_INFO(this->get_logger(), "Go2 Odometry Nav Node started. Publishing /odom and TF: odom -> base_link.");
    }

private:
    void state_callback(const unitree_go::msg::SportModeState::SharedPtr msg) {
        auto current_time = this->now();
        double dt = (current_time - last_time_).seconds();
        
        if (dt <= 0.0 || dt > 0.5) {  // Prevent huge jumps
            last_time_ = current_time;
            return;
        }

        // Raw velocities from Unitree
        double vx_raw = msg->velocity[0];
        double vy_raw = msg->velocity[1];
        double wz_raw = msg->yaw_speed;

        // === 1. LOW-PASS FILTER: Reduce noise ===
        const double alpha = 0.1;  // 0.0 = no trust, 1.0 = full trust in new value
        vx_filtered_ = alpha * vx_raw + (1.0 - alpha) * vx_filtered_;
        vy_filtered_ = alpha * vy_raw + (1.0 - alpha) * vy_filtered_;
        wz_filtered_ = alpha * wz_raw + (1.0 - alpha) * wz_filtered_;

        // === 2. DEADZONE: Zero small values to prevent drift ===
        double vx = std::abs(vx_filtered_) < 0.02 ? 0.0 : vx_filtered_;
        double vy = std::abs(vy_filtered_) < 0.02 ? 0.0 : vy_filtered_;
        double wz = std::abs(wz_filtered_) < 0.01 ? 0.0 : wz_filtered_;

        // === 3. PURE ROTATION: Force vx/vy = 0 if spinning in place ===
        if (std::abs(wz) > 0.1 && std::abs(vx) < 0.05 && std::abs(vy) < 0.05) {
            vx = 0.0;
            vy = 0.0;
        }

        // === 4. INTEGRATE IN WORLD FRAME ===
        double delta_x = (vx * std::cos(yaw_) - vy * std::sin(yaw_)) * dt;
        double delta_y = (vx * std::sin(yaw_) + vy * std::cos(yaw_)) * dt;
        double delta_yaw = wz * dt;

        x_ += delta_x;
        y_ += delta_y;
        yaw_ += delta_yaw;

        // Normalize yaw to [-pi, pi]
        while (yaw_ > M_PI) yaw_ -= 2.0 * M_PI;
        while (yaw_ <= -M_PI) yaw_ += 2.0 * M_PI;

        // === PUBLISH ODOMETRY ===
        auto odom_msg = nav_msgs::msg::Odometry();
        odom_msg.header.stamp = current_time;
        odom_msg.header.frame_id = "odom";
        odom_msg.child_frame_id = "base_link";

        odom_msg.pose.pose.position.x = x_;
        odom_msg.pose.pose.position.y = y_;
        odom_msg.pose.pose.position.z = 0.0;

        tf2::Quaternion q;
        q.setRPY(0, 0, yaw_);
        odom_msg.pose.pose.orientation = tf2::toMsg(q);

        odom_msg.twist.twist.linear.x = vx;
        odom_msg.twist.twist.linear.y = vy;
        odom_msg.twist.twist.angular.z = wz;

        // Add realistic covariance
        odom_msg.pose.covariance[0] = 0.01;
        odom_msg.pose.covariance[7] = 0.01;
        odom_msg.pose.covariance[14] = 99999;  // z
        odom_msg.pose.covariance[21] = 99999;  // roll
        odom_msg.pose.covariance[28] = 99999;  // pitch
        odom_msg.pose.covariance[35] = 0.05;   // yaw

        odom_msg.twist.covariance[0] = 0.01;
        odom_msg.twist.covariance[7] = 0.01;
        odom_msg.twist.covariance[35] = 0.02;

        odom_pub_->publish(odom_msg);

        // === PUBLISH TF: odom -> base_link ===
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = current_time;
        transform.header.frame_id = "odom";
        transform.child_frame_id = "base_link";
        transform.transform.translation.x = x_;
        transform.transform.translation.y = y_;
        transform.transform.translation.z = 0.0;
        transform.transform.rotation = odom_msg.pose.pose.orientation;

        tf_broadcaster_->sendTransform(transform);

        last_time_ = current_time;
    }

    // Member Variables
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr state_sub_;

    double x_, y_, yaw_;
    rclcpp::Time last_time_;

    // Filters
    double wz_filtered_, vx_filtered_, vy_filtered_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Go2OdomNavNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}