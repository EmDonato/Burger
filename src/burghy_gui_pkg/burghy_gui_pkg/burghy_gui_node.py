import requests

import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException

from std_msgs.msg import String
from geometry_msgs.msg import Twist, Twist_stamped
from stm32_nucleo_f303re_driver.msg import Wheelspeed
from nav_msgs.msg import Odometry


from nicegui import ui
import threading

class BurguiGUI(Node):

    def __init__(self):
        super().__init__("burghy_gui_node")

        # Publisher_LLM
        self.publisher_ = self.create_publisher(
            String,
            "ollama/request",
            10
        )

        # Subscriber_LLM
        self.subscription = self.create_subscription(
            String,
            "ollama/response",
            self.listener_callback,
            10
        )
        # Subscriber_odom
        self.subscription = self.create_subscription(
            String,
            "ollama/response",
            self.listener_callback,
            10
        )
        # Subscriber_twist_vel
        self.subscription = self.create_subscription(
            String,
            "ollama/response",
            self.listener_callback,
            10
        )
        # Subscriber_
        self.subscription = self.create_subscription(
            String,
            "ollama/response",
            self.listener_callback,
            10
        )  
        self.history = []

        self.get_logger().info("GUI node started")

    def listener_callback(self, msg: String):

        self.get_logger().info(
            f"Request received: {msg.data}"
        )

        response = self.ask_burgy(msg.data)

        response_msg = String()
        response_msg.data = response

        self.publisher_.publish(response_msg)

        self.get_logger().info(
            "Response published"
        )

    def ask_burgy(self, user_input: str) -> str:

        self.history.append({
            "role": "user",
            "content": user_input
        })

        messages = [
            {
                "role": "system",
                "content": self.system_prompt
            }
        ]

        messages += self.history[-10:]

        try:

            r = requests.post(
                f"{self.base_url}/api/chat",
                json={
                    "model": self.model,
                    "messages": messages,
                    "stream": False,
                    "keep_alive": "5m",
                    "options": {
                        "temperature": 0.85,
                        "num_predict": 80,
                        "num_ctx": 2048,
                        "top_k": 64,
                        "top_p": 0.95
                    }
                },
                timeout=60
            )

            r.raise_for_status()

            data = r.json()

            if "message" not in data:
                return "(no response)"

            reply = data["message"]["content"].strip()

            self.history.append({
                "role": "assistant",
                "content": reply
            })

            return reply if reply else "(Burgy is offline)"

        except requests.exceptions.RequestException as e:

            self.get_logger().error(
                f"Ollama request failed: {e}"
            )

            return "Connection error with Ollama."


def main(args=None):

    rclpy.init(args=args)

    node = BurguiGUI()

    try:

        rclpy.spin(node)

    except (
        KeyboardInterrupt,
        ExternalShutdownException
    ):

        pass

    finally:

        node.destroy_node()

        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()