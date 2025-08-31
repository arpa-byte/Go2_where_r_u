from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    ld = LaunchDescription()

    # This node creates a static link between the 'map' frame (from SLAM)
    # and the 'odom' frame (from robot odometry). It's a common setup.
    map_to_odom_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_pub_map_odom',
        arguments=['0.0', '0.0', '0.0', '0', '0', '0', '1', 'map', 'odom'],
    )

    # This node creates a 'laser_frame' relative to the robot's base.
    # This is where the 2D laser scan data will live.
    base_to_laser_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_pub_laser',
        arguments=['0.0', '0.0', '0.0', '0', '0', '0', '1', 'base_link', 'laser_frame'],
    )

    # This node creates a 'cloud_frame' relative to the robot's base.
    # This is for the 3D point cloud data.
    base_to_cloud_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_pub_cloud',
        arguments=['0.0', '0.0', '0.0', '0', '0', '0', '1', 'base_link', 'cloud_frame'],
    )

    # This is the custom node that the creator made. We will create it in the next step.
    relay_topics_node = Node(
        package='mid360_slam',
        executable='relay_topics',
        name='relay_topics',
        output='screen',
    )

    # Add all the nodes to the launch description
    ld.add_action(map_to_odom_node)
    ld.add_action(base_to_laser_node)
    ld.add_action(base_to_cloud_node)
    ld.add_action(relay_topics_node)

    return ld
