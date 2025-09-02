import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # Get the path to the EKF config file
    pkg_share = get_package_share_directory('go2_localization')
    ekf_config_path = os.path.join(pkg_share, 'config', 'ekf.yaml')

    # Launch the robot_localization EKF node
    robot_localization_node = Node(
       package='robot_localization',
       executable='ekf_node',
       name='ekf_filter_node',
       output='screen',
       parameters=[ekf_config_path]
    )

    return LaunchDescription([
        robot_localization_node
    ])
