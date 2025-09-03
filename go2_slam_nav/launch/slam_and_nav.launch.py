import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # --- GET PACKAGE DIRECTORIES ---
    livox_ros_driver2_dir = get_package_share_directory('livox_ros_driver2')
    mid360_slam_dir = get_package_share_directory('mid360_slam')
    go2_localization_dir = get_package_share_directory('go2_localization')
    go2_slam_nav_dir = get_package_share_directory('go2_slam_nav')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    slam_toolbox_dir = get_package_share_directory('slam_toolbox')

    # --- DEFINE FILE PATHS ---
    livox_config_file = os.path.join(livox_ros_driver2_dir, 'config', 'MID360_config.json')
    nav2_params_file = os.path.join(go2_slam_nav_dir, 'config', 'nav2_params.yaml')
    slam_params_file = os.path.join(mid360_slam_dir, 'config', 'slam_params.yaml')
    rviz_config_file = os.path.join(mid360_slam_dir, 'config', 'slam_config.rviz') # Reusing the SLAM RViz config

    # --- DECLARE LAUNCH ARGUMENTS ---
    network_interface_arg = DeclareLaunchArgument(
        'iface', default_value='enp7s0',
        description='The network interface for Unitree SDK communication.'
    )
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')

    # ========== LAUNCH ACTION DEFINITIONS ==========

    # ----- 1. ROBOT INTERFACE AND SENSORS (from slam.launch.py) -----
    
    odom_bridge_node = Node(
        package='odom_bridge_mid',
        executable='odom_bridge_node_mid',
        name='odom_bridge_node',
        output='screen',
        arguments=[LaunchConfiguration('iface')]
    )

    ekf_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_localization_dir, 'launch', 'ekf.launch.py')
        )
    )

    livox_driver_node = Node(
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

    pointcloud_to_laserscan_node = Node(
        package='pointcloud_to_laserscan',
        executable='pointcloud_to_laserscan_node',
        name='pointcloud_to_laserscan',
        remappings=[('cloud_in', '/livox/lidar'), ('scan', '/scan')],
        parameters=[{
            'target_frame': 'livox_frame',
            'transform_tolerance': 0.5, 'min_height': -1.0, 'max_height': 1.5,
            'angle_min': -3.14159, 'angle_max': 3.14159, 'angle_increment': 0.0087,
            'scan_time': 0.1, 'range_min': 0.3, 'range_max': 40.0,
            'use_inf': True, 'inf_epsilon': 1.0
        }]
    )
    
    scan_qos_relay_node = Node(
        package='scan_qos_relay',
        executable='scan_qos_relay',
        name='scan_qos_relay',
        output='screen'
    )

    static_tf_pub_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_livox_frame',
        arguments=['0', '0', '0.2', '0', '0', '0', 'base_link', 'livox_frame']
    )

    # ----- 2. SLAM TOOLBOX -----
    
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_toolbox_dir, 'launch', 'online_async_launch.py')
        ),
        launch_arguments={'slam_params_file': slam_params_file}.items()
    )

    # ----- 3. NAVIGATION (NAV2) STACK -----
    
    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_bringup_dir, 'launch', 'navigation_launch.py')
        ),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'params_file': nav2_params_file
        }.items()
    )

    # ----- 4. VISUALIZATION (RVIZ) -----
    
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    # --- ASSEMBLE THE FINAL LAUNCH DESCRIPTION ---
    return LaunchDescription([
        network_interface_arg,
        
        # Robot Interface & Sensors
        odom_bridge_node,
        ekf_launch,
        livox_driver_node,
        pointcloud_to_laserscan_node,
        scan_qos_relay_node,
        static_tf_pub_node,
        
        # SLAM
        slam_launch,
        
        # Navigation
        nav2_launch,
        
        # Visualization
        rviz_node
    ])