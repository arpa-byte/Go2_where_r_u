#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <memory>
#include <cmath>

// Unused includes for TF broadcasting have been removed.
// #include <geometry_msgs/msg/transform_stamped.hpp>
// #include <tf2_ros/transform_broadcaster.h>

// The message type from unitree_ros2 we will subscribe to
#include "unitree_go/msg/sport_mode_state.hpp"

class Go2OdomBridgeNode : public rclcpp::Node {
public:
    Go2OdomBridgeNode() : Node("go2_odom_bridge_node") {
        // Publisher for the standard ROS 2 odometry message. This remains.
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("/odom", 50);

        // --- REMOVED ---
        // The TF broadcaster is no longer needed in this node.
        // tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        // Subscriber to the Go2's native state topic
        state_sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
            "/lf/sportmodestate", 10,
            std::bind(&Go2OdomBridgeNode::state_callback, this, std::placeholders::_1));

        // Initialize pose state
        x_ = 0.0;
        y_ = 0.0;
        yaw_ = 0.0;
        last_time_ = this->now();

        // --- UPDATED ---
        // The log message now accurately describes the node's single responsibility.
        RCLCPP_INFO(this->get_logger(), "Go2 Odometry Bridge started. Subscribing to /lf/sportmodestate and publishing to /odom.");
    }

private:
    void state_callback(const unitree_go::msg::SportModeState::SharedPtr msg) {
        auto current_time = this->now();
        double dt = (current_time - last_time_).seconds();
        
        if (dt <= 0.0) {
            return;
        }

        double vx = msg->velocity[0];
        double vy = msg->velocity[1];
        double wz = msg->yaw_speed;

        double delta_x = (vx * std::cos(yaw_) - vy * std::sin(yaw_)) * dt;
        double delta_y = (vx * std::sin(yaw_) + vy * std::cos(yaw_)) * dt;
        double delta_yaw = wz * dt;

        x_ += delta_x;
        y_ += delta_y;
        yaw_ += delta_yaw;

        // --- Publish the Odometry Message ---
        // This is now the node's only output.
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

        odom_pub_->publish(odom_msg);

        // --- REMOVED ---
        // The entire block for creating and broadcasting the TF transform has been deleted.
        // The EKF node is now solely responsible for this.

        last_time_ = current_time;
    }

    // Member Variables
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    // --- REMOVED ---
    // std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr state_sub_;

    double x_, y_, yaw_;
    rclcpp::Time last_time_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Go2OdomBridgeNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}