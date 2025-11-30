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
    go2_driver_dir = get_package_share_directory('go2_driver')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    livox_ros_driver2_dir = get_package_share_directory('livox_ros_driver2')
    go2_localization_dir = get_package_share_directory('go2_localization')

    # --- DEFINE FILE PATHS ---
    map_file = os.path.join(go2_navigation_dir, 'maps', 'map_seven.yaml')
    nav2_params_file = os.path.join(go2_navigation_dir, 'config', 'nav2_params.yaml')
    rviz_config_file = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')
    livox_config_file = os.path.join(livox_ros_driver2_dir, 'config', 'MID360_config.json')

    # --- DECLARE LAUNCH ARGUMENTS ---
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    autostart = LaunchConfiguration('autostart', default='true')

    # --- LAUNCH ACTION DEFINITIONS ---

    # 1. GO2 DRIVER (CRITICAL: cmd_vel, odom, TF, joints)
    go2_driver_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_driver_dir, 'launch', 'go2_driver.launch.py')
        )
    )

    # inside navigation.launch.py, right after go2_driver_launch
    odom_bridge = Node(
        package='go2_odometry_bridge',
        executable='go2_odometry_bridge_node',
        name='odom_bridge_node',
        output='screen'
    )

    # # 2. EKF Launch (fuses go2_driver's /odom + IMU)
    # ekf_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource(
    #         os.path.join(go2_localization_dir, 'launch', 'ekf.launch.py')
    #     )
    # )

    # 3. Livox LiDAR Driver (MID-360)
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
            {'frame_id': 'livox_frame'},
            {'use_sim_time': use_sim_time}
        ]
    )

    # 4. PointCloud to LaserScan (uses go2_driver's /pointcloud)    CHANGES HERE
    pointcloud_to_laserscan = Node(
        package='pointcloud_to_laserscan',
        executable='pointcloud_to_laserscan_node',
        name='pointcloud_to_laserscan_go2',
        remappings=[
            ('cloud_in', '/livox/lidar'),   # <-- CHANGE
            ('scan', '/scan')
        ],
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
        }],
        output='screen'
    )

    # 5. SCAN QoS RELAY – Best-Effort → Reliable for AMCL
    scan_qos_relay = Node(
        package='scan_qos_relay',
        executable='scan_qos_relay',
        name='scan_qos_relay',
        output='screen',
        remappings=[
            ('scan_in', '/scan'),           # ← ADD THIS LINE
            ('scan_out', '/scan_reliable')  # ← ADD THIS LINE
        ]
    )

    # 6. STATIC TRANSFORMS
    tf_livox = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_livox_frame',
        arguments=['0', '0', '0.2', '0', '0', '0', 'base_link', 'livox_frame']
    )

    # 7. Nav2 Bringup
    nav2_bringup_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')
        ),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'autostart': autostart,
            'params_file': nav2_params_file,
            'map': map_file,
            'scan_topic': '/scan_reliable',
            'transform_tolerance': '1.0',
            # Auto-set initial pose at (0, 0, 0)
            'initial_pose': {
                'x': 0.0,
                'y': 0.0,
                'z': 0.0,
                'yaw': 0.0
            }
        }.items(),
    )

    # 8. RViz
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    # --- ASSEMBLE LAUNCH DESCRIPTION ---
    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('autostart', default_value='true'),

        # 1. Core robot interface
        go2_driver_launch,

        odom_bridge,

        # 2. Localization
        #ekf_launch,

        # 3. Sensors
        livox_driver,
        pointcloud_to_laserscan,

        # 4. QoS fix
        scan_qos_relay,

        # 5. TF
        tf_livox,

        # 6. Navigation
        nav2_bringup_launch,

        # 7. Visualization
        rviz_node
    ])