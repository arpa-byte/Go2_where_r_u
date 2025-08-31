import rclpy
from rclpy.node import Node

class RelayNode(Node):
    def __init__(self):
        super().__init__('relay_topics_node')
        self.get_logger().info('Relay topics node has started successfully.')
        # In the future, subscriber and publisher logic will go here.

def main(args=None):
    rclpy.init(args=args)
    node = RelayNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
