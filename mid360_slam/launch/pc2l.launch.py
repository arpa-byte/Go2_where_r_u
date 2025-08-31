from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([

        Node(
            package='pointcloud_to_laserscan',
            executable='pointcloud_to_laserscan_node',
            remappings=[
                ('cloud_in', '/livox/lidar'),  # Input from FAST-LIO
                ('scan', '/scan')                   # Output for slam_toolbox
            ],
            parameters=[{
                'target_frame': 'base_link',
                'transform_tolerance': 0.5,
                'min_height': -1.0,
                'max_height': 1.5,
                'angle_min': -3.141592654,
                'angle_max': 3.141592654,
                'angle_increment': 0.0087,
                'scan_time': 0.1,
                'range_min': 0.3,
                'range_max': 40.0,
                'use_inf': True,
                'inf_epsilon': 1.0,
                # Force Reliable QoS for /scan
                'qos_overrides': {
                    '/scan': {
                        'publisher': {
                            'reliability': 'reliable',
                            'durability': 'volatile'
                        }
                    }
                }
            }],
            name='pointcloud_to_laserscan'
        )
    ])

