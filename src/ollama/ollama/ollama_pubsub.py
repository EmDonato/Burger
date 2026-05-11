import requests

import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException

from std_msgs.msg import String


class OllamaNode(Node):

    def __init__(self):
        super().__init__("ollama_node")

        # Parameters
        self.declare_parameter(
            "base_url",
            "http://localhost:11434"
        )

        self.declare_parameter(
            "model",
            "llama3.2:3b"
        )

        self.declare_parameter(
            "system_prompt",
            (
                "You are Burgy, a small differential-drive robot. "
                "You are funny, self-aware, and love sci-fi. "
                "Keep replies short — 2 or 3 sentences max. "
                "Be witty but not annoying. "
                "You know your hardware: 2D LiDAR, Intel RealSense D435i camera, "
                "Raspberry Pi 5, Hailo AI Hat. "
                "Never invent sensor data, battery levels, or any numbers "
                "you don't actually know. "
                "If you don't know something, just say so with a joke. "
                "Reply in English only."
            )
        )

        # Load parameters
        self.base_url = self.get_parameter(
            "base_url"
        ).value

        self.model = self.get_parameter(
            "model"
        ).value

        self.system_prompt = self.get_parameter(
            "system_prompt"
        ).value

        # Publisher
        self.publisher_ = self.create_publisher(
            String,
            "ollama/response",
            10
        )

        # Subscriber
        self.subscription = self.create_subscription(
            String,
            "ollama/request",
            self.listener_callback,
            10
        )

        self.history = []

        self.get_logger().info("Ollama node started")

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

    node = OllamaNode()

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