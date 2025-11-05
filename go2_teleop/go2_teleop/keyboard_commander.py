import rclpy
from rclpy.node import Node
import sys, select, termios, tty
import json
# No std_msgs.Header needed
from unitree_api.msg import Request, RequestHeader, Identity, Lease, Policy  # Add these; if in unitree_go.msg, change accordingly

# --- User-configurable settings ---
FORWARD_SPEED = 0.5  # m/s
BACKWARD_SPEED = -0.3 # m/s
ROTATION_SPEED = 0.8 # rad/s
# ---

msg = """
Control Your Go2 via ROS 2 Topics!
---------------------------
Moving around:
        w
   a    s    d
        x

w/s : move forward/backward
a/d : turn left/right
x : stop (sends zero velocity)

Press CTRL-C to quit
"""

def getKey(settings):
    tty.setraw(sys.stdin.fileno())
    rlist, _, _ = select.select([sys.stdin], [], [], 0.1)
    if rlist:
        key = sys.stdin.read(1)
    else:
        key = ''
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
    return key

class KeyboardCommanderNode(Node):
    def __init__(self):
        super().__init__('go2_keyboard_commander')
        # Publisher for motion commands
        self.sport_pub = self.create_publisher(Request, '/api/sport/request', 10)
        # --- ADDED: Publisher for the lease ---
        self.lease_pub = self.create_publisher(Request, '/api/sport_lease/request', 10)
        
        self.get_logger().info("Go2 Keyboard Commander is running.")
        print(msg)

        self.request_id = 0  # Incremental unique ID for requests

        # --- ADDED: Timer to periodically renew the lease ---
        self.lease_timer = self.create_timer(2.0, self.request_lease)
        
        # --- ADDED: Request the lease immediately on startup ---
        self.request_lease()

    def send_sport_command(self, x_vel, yaw_vel):
        self.request_id += 1
        req_msg = Request()
        req_msg.header = RequestHeader()
        req_msg.header.identity = Identity(id=self.request_id, api_id=1008)  # 1008 for velocity/move commands
        req_msg.header.lease = Lease(id=0)
        req_msg.header.policy = Policy(priority=0, noreply=False)
        command_params = {
            'mode': 2, 'gait_type': 1, 'velocity': [x_vel, 0.0],
            'yaw_speed': yaw_vel, 'foot_raise_height': 0.08
        }
        req_msg.parameter = json.dumps(command_params)
        req_msg.binary = []  # Empty binary data
        self.sport_pub.publish(req_msg)
        self.get_logger().info(f"Published command: x_vel={x_vel}, yaw_vel={yaw_vel}")
    
    def request_lease(self):
        self.request_id += 1
        req_msg = Request()
        req_msg.header = RequestHeader()
        req_msg.header.identity = Identity(id=self.request_id, api_id=1001)  # Guess for lease; adjust if needed (monitor /api/sport_lease/response)
        req_msg.header.lease = Lease(id=0)
        req_msg.header.policy = Policy(priority=0, noreply=False)
        req_msg.parameter = json.dumps("lease")
        req_msg.binary = []
        self.lease_pub.publish(req_msg)
        self.get_logger().info("Lease request sent.")

    def release_lease(self):
        self.request_id += 1
        req_msg = Request()
        req_msg.header = RequestHeader()
        req_msg.header.identity = Identity(id=self.request_id, api_id=1001)  # Same as request
        req_msg.header.lease = Lease(id=0)
        req_msg.header.policy = Policy(priority=0, noreply=False)
        req_msg.parameter = json.dumps("release")
        req_msg.binary = []
        self.lease_pub.publish(req_msg)
        self.get_logger().info("Lease released.")

def main(args=None):
    rclpy.init(args=args)
    settings = termios.tcgetattr(sys.stdin)
    node = KeyboardCommanderNode()
    
    try:
        while rclpy.ok():
            key = getKey(settings)
            if key == 'w':
                node.send_sport_command(FORWARD_SPEED, 0.0)
            elif key == 's':
                node.send_sport_command(BACKWARD_SPEED, 0.0)
            elif key == 'a':
                node.send_sport_command(0.0, ROTATION_SPEED)
            elif key == 'd':
                node.send_sport_command(0.0, -ROTATION_SPEED)
            elif key == 'x':
                node.send_sport_command(0.0, 0.0)
            else:
                # This ensures the robot stops if no key is pressed for a while
                node.send_sport_command(0.0, 0.0)
                if (key == '\x03'): # CTRL-C
                    break
    finally:
        node.send_sport_command(0.0, 0.0) # Send a final stop command
        # --- ADDED: Release the lease before shutting down ---
        node.release_lease()
        
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()