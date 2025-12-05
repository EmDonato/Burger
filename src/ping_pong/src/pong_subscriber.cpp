#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class PongSubscriber : public rclcpp::Node {
public:
    PongSubscriber() : Node("pong_subscriber") {
        sub_ = this->create_subscription<std_msgs::msg::String>(
            "ping", 10,
            [this](const std_msgs::msg::String::SharedPtr msg){
                RCLCPP_INFO(this->get_logger(), "Ricevuto: %s", msg->data.c_str());
                RCLCPP_INFO(this->get_logger(), "Risposta: pong");
            }
        );
    }
private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PongSubscriber>());
    rclcpp::shutdown();
}
