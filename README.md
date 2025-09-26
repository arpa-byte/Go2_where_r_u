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

## Package Info
arpan@legion-y540-ubuntu:~/unitree2/mid360_nav_only_clone/src$ ll
total 52
drwxrwxr-x 13 arpan arpan 4096 Sep 21 19:47 ./
drwxrwxr-x  6 arpan arpan 4096 Sep 21 21:32 ../
drwxrwxr-x  4 arpan arpan 4096 Sep 11 19:33 go2_control_bridge/
drwxrwxr-x  6 arpan arpan 4096 Sep 10 18:06 go2_localization/
drwxrwxr-x  7 arpan arpan 4096 Sep 10 18:06 go2_navigation/
drwxrwxr-x  4 arpan arpan 4096 Sep 21 19:49 go2_odometry_bridge/
drwxrwxr-x  6 arpan arpan 4096 Sep 10 18:06 go2_slam_nav/
drwxrwxr-x  5 arpan arpan 4096 Sep 14 15:15 go2_teleop/
drwxrwxr-x  7 arpan arpan 4096 Sep 10 18:06 mid360_slam/
drwxrwxr-x  4 arpan arpan 4096 Sep 10 18:06 odom_bridge_mid/
drwxrwxr-x  4 arpan arpan 4096 Sep 10 18:06 odom_logger/
drwxrwxr-x  4 arpan arpan 4096 Sep 10 18:06 odom_tester_mid/
drwxrwxr-x  5 arpan arpan 4096 Sep 10 18:06 scan_qos_relay/


go2_control_bridge - rejected
go2_localization	| was being used with odom bridge		| now obselete
go2_navigation		| guided navigation 				| path planning done | 		
go2_odometry_bridge 	| ros2 command to sdk2 command conversion 	| obselete (now using the official ros packages)
go2_slam_nav		| autonomous mapping | in progress 		| path planning issues
go2_teleop		| bridge teleop test 				| obselete	
mid360_slam		| mapping package				| successfully working 	| minor odom issues

Running mid360_slam (for mapping

source ~/.bashrc (to source livox packages and unitree_ros2 package setup.sh)
source install/setup.bash
ros2 launch mid360_slam slam.launch.py 				| obselete
ros2 launch mid360_slam slam.launch.py iface:=enp7s0		| Removed hard coded network requirement



Running go2_navigation (for guided navigation)

source ~/.bashrc (to source livox packages and unitree_ros2 package setup.sh)
source install/setup.bash
ros2 launch go2_navigation navigation.launch.py 




arpan@legion-y540-ubuntu:~/unitree2/mid360_nav_only_clone/src$ tree
.
├── go2_control_bridge
│   ├── CMakeLists.txt
│   ├── include
│   │   └── go2_control_bridge
│   ├── package.xml
│   └── src
│       └── control_bridge_node.cpp
├── go2_localization
│   ├── CMakeLists.txt
│   ├── config
│   │   └── ekf.yaml
│   ├── include
│   │   └── go2_localization
│   ├── launch
│   │   └── ekf.launch.py
│   ├── package.xml
│   └── src
├── go2_navigation
│   ├── CMakeLists.txt
│   ├── config
│   │   └── nav2_params.yaml
│   ├── include
│   │   └── go2_navigation
│   ├── launch
│   │   └── navigation.launch.py
│   ├── maps
│   │   ├── my_map.pgm
│   │   ├── my_map.yaml
│   │   ├── my_second_map.pgm
│   │   └── my_second_map.yaml
│   ├── package.xml
│   └── src
├── go2_odometry_bridge
│   ├── CMakeLists.txt
│   ├── include
│   │   └── go2_odometry_bridge
│   ├── package.xml
│   └── src
│       └── go2_odom_bridge_node.cpp
├── go2_slam_nav
│   ├── CMakeLists.txt
│   ├── config
│   │   └── nav2_params.yaml
│   ├── include
│   │   └── go2_slam_nav
│   ├── launch
│   │   └── slam_and_nav.launch.py
│   ├── package.xml
│   └── src
├── go2_teleop
│   ├── go2_teleop
│   │   ├── __init__.py
│   │   └── keyboard_commander.py
│   ├── package.xml
│   ├── resource
│   │   └── go2_teleop
│   ├── setup.cfg
│   ├── setup.py
│   └── test
│       ├── test_copyright.py
│       ├── test_flake8.py
│       └── test_pep257.py
├── mid360_slam
│   ├── config
│   │   ├── slam_config.rviz
│   │   └── slam_params.yaml
│   ├── launch
│   │   ├── pc2l.launch.py
│   │   ├── relay_topics.launch.py
│   │   └── slam.launch.py
│   ├── mid360_slam
│   │   ├── __init__.py
│   │   └── relay_topic.py
│   ├── package.xml
│   ├── resource
│   │   └── mid360_slam
│   ├── setup.cfg
│   ├── setup.py
│   └── test
│       ├── test_copyright.py
│       ├── test_flake8.py
│       └── test_pep257.py
├── odom_bridge_mid
│   ├── CMakeLists.txt
│   ├── include
│   │   └── odom_bridge_mid
│   ├── package.xml
│   └── src
│       └── odom_bridge_node_mid.cpp
├── odom_logger
│   ├── CMakeLists.txt
│   ├── include
│   │   └── odom_logger
│   ├── package.xml
│   └── src
│       └── odom_logger_node.cpp
├── odom_tester_mid
│   ├── CMakeLists.txt
│   ├── include
│   │   └── odom_tester_mid
│   ├── package.xml
│   └── src
│       └── odom_tester_node.cpp
└── scan_qos_relay
    ├── package.xml
    ├── resource
    │   └── scan_qos_relay
    ├── scan_qos_relay
    │   ├── __init__.py
    │   └── relay_node.py
    ├── setup.cfg
    ├── setup.py
    └── test
        ├── test_copyright.py
        ├── test_flake8.py
        └── test_pep257.py

53 directories, 63 files
arpan@legion-y540-ubuntu:~/unitree2/mid360_nav_only_clone/src$
