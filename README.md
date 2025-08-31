# Go2_where_r_u: 2D SLAM with Unitree Go2 and Livox MID-360

This repository contains the ROS2 Humble packages required to perform 2D SLAM using a Unitree Go2 robot equipped with a Livox MID-360 LiDAR.

## System Overview

This project integrates the robot's built-in odometry with the LiDAR's 3D point cloud data, which is converted to a 2D laser scan for use with `slam_toolbox`.

- **`odom_bridge_mid`**: A C++ node that uses the `unitree_sdk2` to subscribe to the robot's internal state and publishes the `odom` -> `base_link` transform and a `nav_msgs/msg/Odometry` topic.
- **`mid360_slam`**: The main package containing launch files, RViz configurations, and SLAM parameters.
- **`scan_qos_relay`**: A simple Python utility to ensure the laser scan topic has a "reliable" QoS for `slam_toolbox`.
- **`odom_tester_mid`**: A utility package for debugging and verifying the connection to the robot's odometry data.

## Hardware Requirements

- Unitree Go2 Edu
- Livox MID-360 LiDAR
- Ubuntu 22.04 Laptop

## Software Dependencies

- ROS2 Humble
- `unitree_sdk2` and `unitree_ros2`
- `livox_ros_driver2`
- `slam_toolbox`: `sudo apt install ros-humble-slam-toolbox`
- `pointcloud_to_laserscan`: `sudo apt install ros-humble-pointcloud-to-laserscan`

##Package Dependencies
Make sure these packages are installed in the unitree workspace.
- unitree_sdk2 (https://github.com/unitreerobotics/unitree_sdk2)
- livox packages (https://github.com/Livox-SDK/livox_ros2_driver)
- fast Lio packages (https://github.com/hku-mars/FAST_LIO/tree/ROS2)

## How to Run

1.  Source the required workspaces (especially `ws_livox` for the LiDAR driver).
    ```bash
    source ~/unitree2/ws_livox/install/setup.bash
    source <path_to_this_workspace>/install/setup.bash
    ```
2.  Launch the main SLAM system.
    ```bash
    # Replace 'enp7s0' with your robot's network interface if different
    ros2 launch mid360_slam slam.launch.py iface:=enp7s0
    ```
