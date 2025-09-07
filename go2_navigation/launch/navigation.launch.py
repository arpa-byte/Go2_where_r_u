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
    
    # Re-use robot interface launch from our other package
    mid360_slam_dir = get_package_share_directory('mid360_slam')

    # --- DEFINE FILE PATHS ---
    map_file = os.path.join(go2_navigation_dir, 'maps', 'my_map.yaml')
    nav2_params_file = os.path.join(go2_navigation_dir, 'config', 'nav2_params.yaml')
    rviz_config_file = os.path.join(nav2_bringup_dir, 'rviz', 'nav2_default_view.rviz')

    # --- DECLARE LAUNCH ARGUMENTS ---
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    autostart = LaunchConfiguration('autostart', default='true')

    # --- LAUNCH ACTION DEFINITIONS ---
    
    # 1. LAUNCH ROBOT INTERFACES
    # We include our previous launch file but disable SLAM and RViz
    robot_interface_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(mid360_slam_dir, 'launch', 'slam.launch.py')
        ),
        launch_arguments={
            'rviz_enable': 'false',
            'slam_toolbox_enable': 'false'
        }.items()
    )

    # 2. LAUNCH NAV2 BRINGUP
    # This is the main Nav2 launch file. It will start map_server, amcl, and the other servers.
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

    # 3. LAUNCH RVIZ
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )
    
    # --- ASSEMBLE THE FINAL LAUNCH DESCRIPTION ---
    return LaunchDescription([
        robot_interface_launch,
        nav2_bringup_launch,
        rviz_node
    ])
