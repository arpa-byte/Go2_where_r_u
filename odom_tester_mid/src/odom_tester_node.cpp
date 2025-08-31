#include <rclcpp/rclcpp.hpp>
#include <iomanip>
#include <iostream>

// Unitree SDK Includes - These are essential for communicating with the robot
#include "unitree/robot/channel/channel_subscriber.hpp"
#include "unitree/robot/channel/channel_factory.hpp"
#include "unitree/idl/go2/SportModeState_.hpp"

class OdomTesterNode : public rclcpp::Node
{
public:
    // The constructor takes the network interface name as an argument
    OdomTesterNode(const std::string& interface_name) : Node("odom_tester_node")
    {
        RCLCPP_INFO(this->get_logger(), "Odometry tester node started.");

        // Initialize the Unitree ChannelFactory to connect to the robot's internal DDS
        unitree::robot::ChannelFactory::Instance()->Init(0, interface_name);

        // Create a subscriber for the "rt/sportmodestate" topic from the robot's SDK
        sport_mode_subscriber_ = std::make_shared<unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>>(
            "rt/sportmodestate");
        
        // Initialize the channel and set our callback function
        sport_mode_subscriber_->InitChannel([this](const void* message) {
            this->SportModeCallback(message);
        });
    }

private:
    void SportModeCallback(const void* message)
    {
        // Cast the incoming message to the correct Unitree DDS message type
        auto state = static_cast<const unitree_go::msg::dds_::SportModeState_*>(message);

        // Access the relevant data fields from the message
        const auto& position = state->position();
        const auto& orientation_quat = state->imu_state().quaternion();
        const auto& linear_velocity = state->velocity();
        const auto& angular_velocity = state->imu_state().gyroscope();

        // Print the received data to the console
        RCLCPP_INFO(this->get_logger(), "--- ODOMETRY DATA RECEIVED ---");
        std::cout << std::fixed << std::setprecision(4)
                  << "Position (x, y, z):         [" << position[0] << ", " << position[1] << ", " << position[2] << "]" << std::endl
                  // Unitree quaternion order is [w, x, y, z]
                  << "Orientation (x, y, z, w):   [" << orientation_quat[1] << ", " << orientation_quat[2] << ", " << orientation_quat[3] << ", " << orientation_quat[0] << "]" << std::endl
                  << "Linear Velocity (x, y, z):  [" << linear_velocity[0] << ", " << linear_velocity[1] << ", " << linear_velocity[2] << "]" << std::endl
                  << "Angular Velocity (x, y, z): [" << angular_velocity[0] << ", " << angular_velocity[1] << ", " << angular_velocity[2] << "]" << std::endl
                  << std::endl;
    }

    // Member variable to hold the subscriber
    std::shared_ptr<unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>> sport_mode_subscriber_;
};

int main(int argc, char** argv)
{
    // The program requires a network interface name (e.g., 'eth0' or 'wlan0') to run
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " network_interface" << std::endl;
        return 1;
    }

    rclcpp::init(argc, argv);
    auto node = std::make_shared<OdomTesterNode>(argv[1]);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
