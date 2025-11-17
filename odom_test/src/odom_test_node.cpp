#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include "unitree_go/msg/sport_mode_state.hpp"
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>   // <-- this header

class OdomTestNode : public rclcpp::Node {
public:
    OdomTestNode() : Node("odom_test_node") {
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
            "/lf/sportmodestate", 10,
            std::bind(&OdomTestNode::callback, this, std::placeholders::_1));

        x_ = y_ = yaw_ = 0.0;
        last_time_ = this->now();

        RCLCPP_INFO(this->get_logger(), "odom_test_node started. Publishing /odom + TF (odom→base_link)");
    }

private:
    void callback(const unitree_go::msg::SportModeState::SharedPtr msg) {
        auto current_time = this->now();
        double dt = (current_time - last_time_).seconds();
        if (dt <= 0.0) return;

        double vx = msg->velocity[0];
        double vy = msg->velocity[1];
        double wz = msg->yaw_speed;

        // Dead reckoning in robot frame
        double delta_x = (vx * cos(yaw_) - vy * sin(yaw_)) * dt;
        double delta_y = (vx * sin(yaw_) + vy * cos(yaw_)) * dt;
        double delta_yaw = wz * dt;

        x_ += delta_x;
        y_ += delta_y;
        yaw_ += delta_yaw;

        // Publish Odometry
        auto odom = nav_msgs::msg::Odometry();
        odom.header.stamp = current_time;
        odom.header.frame_id = "odom";
        odom.child_frame_id = "base_link";
        odom.pose.pose.position.x = x_;
        odom.pose.pose.position.y = y_;
        odom.pose.pose.position.z = 0.0;
        tf2::Quaternion q;
        q.setRPY(0, 0, yaw_);
        odom.pose.pose.orientation = tf2::toMsg(q);
        odom.twist.twist.linear.x = vx;
        odom.twist.twist.linear.y = vy;
        odom.twist.twist.angular.z = wz;
        odom_pub_->publish(odom);

        // Publish TF
        geometry_msgs::msg::TransformStamped tf;
        tf.header.stamp = current_time;
        tf.header.frame_id = "odom";
        tf.child_frame_id = "base_link";
        tf.transform.translation.x = x_;
        tf.transform.translation.y = y_;
        tf.transform.translation.z = 0.0;
        tf.transform.rotation = odom.pose.pose.orientation;
        tf_broadcaster_->sendTransform(tf);

        last_time_ = current_time;
    }

    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr sub_;

    double x_, y_, yaw_;
    rclcpp::Time last_time_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OdomTestNode>());
    rclcpp::shutdown();
    return 0;
}