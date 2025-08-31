import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
from sensor_msgs.msg import LaserScan


class ScanQoSRelay(Node):
    def __init__(self):
        super().__init__('scan_qos_relay')

        # Subscriber QoS: Best Effort (to match pointcloud_to_laserscan output)
        sub_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        # Publisher QoS: Reliable (for slam_toolbox)
        pub_qos = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        self.sub = self.create_subscription(
            LaserScan,
            '/scan',
            self.scan_callback,
            qos_profile=sub_qos
        )

        self.pub = self.create_publisher(
            LaserScan,
            '/scan_reliable',
            qos_profile=pub_qos
        )

        self.get_logger().info('Scan QoS Relay started: /scan → /scan_reliable')

    def scan_callback(self, msg):
        self.pub.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = ScanQoSRelay()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()


