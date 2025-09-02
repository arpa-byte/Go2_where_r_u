import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription # <-- ADD THIS IMPORT
from launch.launch_description_sources import PythonLaunchDescriptionSource # <-- AND THIS ONE


def generate_launch_description():
    
    # --- File Paths ---
    # NOTE: We must find the livox_ros_driver2 package, so you must have sourced
    # your ws_livox workspace BEFORE launching this file.
    livox_ros_driver2_dir = get_package_share_directory('livox_ros_driver2')
    mid360_slam_dir = get_package_share_directory('mid360_slam')
    go2_localization_dir = get_package_share_directory('go2_localization') # <-- ADD THIS LINE


    livox_config_file = os.path.join(livox_ros_driver2_dir, 'config', 'MID360_config.json')
    slam_params_file = os.path.join(mid360_slam_dir, 'config', 'slam_params.yaml')
    #rviz_config_file = os.path.join(livox_ros_driver2_dir, 'config', 'display_point_cloud_ROS2.rviz')
    rviz_config_file = os.path.join(mid360_slam_dir, 'config', 'slam_config.rviz')

    # Define the network interface for the robot. Change 'eth0' if yours is different.
    network_interface = 'enp7s0'

    # --- Node Definitions ---

    # Terminal 1: Odom Bridge - THE NEW, CRITICAL NODE
    # This node connects to the robot, gets odometry data, and publishes the odom->base_link transform.
    odom_bridge = Node(
        package='odom_bridge_mid',
        executable='odom_bridge_node_mid',
        name='odom_bridge_node',
        output='screen',
        arguments=[network_interface] # Pass the network interface as a command-line argument
    )

    ekf_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_localization_dir, 'launch', 'ekf.launch.py')
        )
    )

    # Terminal 2: Livox LiDAR Driver
    livox_driver = Node(
        package='livox_ros_driver2',
        executable='livox_ros_driver2_node',
        name='livox_lidar_publisher',
        output='screen',
        parameters=[
            {'publish_freq': 10.0},
            {'xfer_format': 0},
            {'multi_topic': 0},
            {'data_src': 0},
            {'user_config_path': livox_config_file},
            {'frame_id': 'livox_frame'}
        ]
    )

    # Terminal 3: PointCloud to LaserScan Converter
    pointcloud_to_laserscan = Node(
        package='pointcloud_to_laserscan',
        executable='pointcloud_to_laserscan_node',
        name='pointcloud_to_laserscan',
        remappings=[('cloud_in', '/livox/lidar'),
                    ('scan', '/scan')],
        parameters=[{
            'target_frame': 'livox_frame',
            'transform_tolerance': 0.5,
            'min_height': -1.0,
            'max_height': 1.5,
            'angle_min': -3.14159,
            'angle_max': 3.14159,
            'angle_increment': 0.0087,
            'scan_time': 0.1,
            'range_min': 0.3,
            'range_max': 40.0,
            'use_inf': True,
            'inf_epsilon': 1.0
        }]
    )
    
    # Terminal 4: Scan QoS Relay
    scan_qos_relay = Node(
        package='scan_qos_relay',
        executable='scan_qos_relay',
        name='scan_qos_relay',
        output='screen'
    )

    # Terminal 5: Static Transform Publisher (Robot Base to LiDAR)
    # IMPORTANT: Adjust the '0.2' to the actual measured height of your LiDAR sensor.
    static_tf_pub = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_livox_frame',
        arguments=['0', '0', '0.2', '0', '0', '0', 'base_link', 'livox_frame']
    )

    # Terminal 6: SLAM Toolbox
    slam_toolbox = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        parameters=[
            slam_params_file,
            {'use_sim_time': False}
        ],
        remappings=[('scan', '/scan_reliable')]
    )

    # Terminal 7: RViz2
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file]
    )

    # --- Launch Description ---
    return LaunchDescription([
        odom_bridge,
        ekf_launch,
        livox_driver,
        pointcloud_to_laserscan,
        scan_qos_relay,
        static_tf_pub,
        slam_toolbox,
        rviz
    ])