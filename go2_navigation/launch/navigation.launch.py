import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # --- GET PACKAGE DIRECTORIES ---
    go2_navigation_dir = get_package_share_directory('go2_navigation')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    livox_ros_driver2_dir = get_package_share_directory('livox_ros_driver2')
    go2_localization_dir = get_package_share_directory('go2_localization')

    # --- DEFINE FILE PATHS ---
    map_file = os.path.join(go2_navigation_dir, 'maps', 'map_four.yaml')
    nav2_params_file = os.path.join(go2_navigation_dir, 'config', 'nav2_params.yaml')
    rviz_config_file = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    livox_config_file = os.path.join(livox_ros_driver2_dir, 'config', 'MID360_config.json')

    # --- DECLARE LAUNCH ARGUMENTS ---
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    autostart = LaunchConfiguration('autostart', default='true')

    # --- LAUNCH ACTION DEFINITIONS ---

    # Odom Bridge Node
    odom_bridge = Node(
        package='go2_odometry_bridge',
        executable='go2_odometry_bridge_node',
        name='odom_bridge_node',
        output='screen'
    )

    # EKF Launch Include
    ekf_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_localization_dir, 'launch', 'ekf.launch.py')
        )
    )

    # Livox LiDAR Driver Node
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

    # PointCloud to LaserScan Converter Node
    #pointcloud_to_laserscan = Node(
    #    package='pointcloud_to_laserscan',
    #    executable='pointcloud_to_laserscan_node',
    #    name='pointcloud_to_laserscan',
    #    remappings=[('cloud_in', '/livox/lidar'), ('scan', '/scan')],
    #    parameters=[{
    #        'target_frame': 'livox_frame',
    #        'transform_tolerance': 0.5,
    #        'min_height': -1.0,
    #        'max_height': 1.5,
    #        'angle_min': -3.14159,
    #        'angle_max': 3.14159,
    #        'angle_increment': 0.0087,
    #        'scan_time': 0.1,
    #        'range_min': 0.3,
    #        'range_max': 40.0,
    #        'use_inf': True,
    #        'inf_epsilon': 1.0
    #    }]
    #)

    # Scan QoS Relay Node
    #scan_qos_relay = Node(
    #    package='scan_qos_relay',
    #    executable='scan_qos_relay',
    #    name='scan_qos_relay',
    #    output='screen'
    #)

    # Static Transform Publisher Node
    static_tf_pub = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_livox_frame',
        arguments=['0', '0', '0.2', '0', '0', '0', 'base_link', 'livox_frame']
    )

    # Nav2 Bringup Launch
    nav2_bringup_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
        ),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'autostart': autostart,
            'params_file': nav2_params_file,
            'map': map_file
        }.items(),
    )

    # In navigation.launch.py, add:
    radar_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_radar',
        arguments=['0', '0', '0.2', '0', '0', '0', 'base_link', 'radar']  # Adjust Z if needed
    )

    # RViz Node
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    # --- ASSEMBLE THE FINAL LAUNCH DESCRIPTION ---
    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('autostart', default_value='true'),
        odom_bridge,
        ekf_launch,
        livox_driver,
        #pointcloud_to_laserscan,
        radar_tf,
        #scan_qos_relay,
        static_tf_pub,
        nav2_bringup_launch,
        rviz_node
    ])