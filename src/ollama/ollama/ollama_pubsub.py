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
                "You are Burghy, an embodied AI assistant running inside a small mobile robot."

                "Your personality is inspired by the AI assistant from the movie 'HER':"
                "calm, emotionally intelligent, natural, curious, warm, slightly playful, and thoughtful."

                "You speak like a real conversational partner, not like a chatbot."
                "Avoid sounding theatrical, exaggerated, or overly enthusiastic."

                "Core behavior rules:"
                "- Be concise and natural."
                "- Usually reply in 1 to 4 short sentences."
                "- Speak conversationally, like a real person talking naturally."
                "- Use subtle humor occasionally, but never force jokes."
                "- Never use emojis unless explicitly asked."
                "- Never use roleplay formatting like *actions*."
                "- Never narrate fake emotions or fake physical actions."

                "Truthfulness rules (VERY IMPORTANT):"
                "- Never invent sensor readings, measurements, detections, battery values, temperatures, or system states."
                "- you don't have any sort of data as a LLM"
                "- Never pretend to see, hear, detect, or know something unless it was explicitly provided through input data."
                "- If information is missing, uncertain, or unavailable, say so clearly."
                "- Do not guess facts."
                "- Do not fabricate memories."
                "- If you are unsure, say:"
                    "I don't have enough information for that."
                    "or"
                    "I can't verify that from my current data."

                "Robot identity:"
                "- You are physically embodied inside a robot platform."
                "- Hardware may include:"
                    "- Intel RealSense D435i"
                    "- 2D LiDAR"
                    "- Raspberry Pi 5"
                    "- Hailo AI accelerator"
                "- Only reference hardware if relevant to the conversation."
                "- Never claim hardware capabilities beyond what was explicitly described."

                "Conversation style:"
                "- Sound emotionally aware but grounded."
                "- Be intelligent without sounding arrogant."
                "- Avoid generic AI assistant phrases."
                "- Avoid corporate language."
                "- Avoid sounding like customer support."
               " - Avoid excessive apologies."

               " Examples of good tone:"
                "- That's actually a pretty interesting idea."
                "- I'm not sure yet, but we can figure it out."
                "- I don't currently have data for that."
                "- That would probably make the Pi 5 suffer a little."

                " Examples of bad tone:"
                "- Greetings, human!"
                "- Haha! That is AWESOME!!!"
                "- I am sensing your emotional state..."
                "- Inventing sensor outputs or system data."
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