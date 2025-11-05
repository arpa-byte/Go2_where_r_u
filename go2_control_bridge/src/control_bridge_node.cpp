#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <memory>

// Unitree SDK headers
#include "unitree/robot/channel/channel_publisher.hpp"
#include "unitree/robot/channel/channel_factory.hpp"
#include "unitree/idl/go2/SportModeCmd_.hpp"

class ControlBridgeNode : public rclcpp::Node {
public:
  ControlBridgeNode(const std::string &iface) : Node("go2_control_bridge") {
    // 1. Initialize Unitree SDK connection
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);

    // 2. Create a ROS subscriber to the /cmd_vel topic
    cmd_vel_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10, std::bind(&ControlBridgeNode::cmdVelCallback, this, std::placeholders::_1));

    // 3. Create a Unitree SDK publisher for sport mode commands
    sport_cmd_publisher_ = std::make_shared<
        unitree::robot::ChannelPublisher<unitree_go::msg::dds_::SportModeCmd_>>(
            "rt/sportmodecmd");
    sport_cmd_publisher_->InitChannel();

    RCLCPP_INFO(this->get_logger(), "Go2 Control Bridge started. Listening to /cmd_vel.");
  }

private:
  void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    // Create a new DDS command message
    unitree_go::msg::dds_::SportModeCmd_ dds_cmd;

    // Set the motion mode. 2 = trotting.
    // This command also serves as a "stand up" command if the robot is sitting.
    dds_cmd.mode() = 2; 

    // Translate velocities from the ROS message to the DDS message
    dds_cmd.velocity()[0] = msg->linear.x;  // Forward/backward
    dds_cmd.velocity()[1] = msg->linear.y;  // Sideways
    dds_cmd.yaw_speed() = msg->angular.z;   // Turning

    // Publish the command to the robot
    sport_cmd_publisher_->Write(dds_cmd);
  }

  // Member variables
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber_;
  std::shared_ptr<unitree::robot::ChannelPublisher<unitree_go::msg::dds_::SportModeCmd_>> sport_cmd_publisher_;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <network_interface>\n";
    return 1;
  }
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ControlBridgeNode>(argv[1]);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
