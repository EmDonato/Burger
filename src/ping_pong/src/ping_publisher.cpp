#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class PingPublisher : public rclcpp::Node {
public:
    PingPublisher() : Node("ping_publisher") {
        pub_ = this->create_publisher<std_msgs::msg::String>("ping", 10);
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            [this]() {
                std_msgs::msg::String msg;
                msg.data = "ping";
                pub_->publish(msg);
            }
        );
    }
private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PingPublisher>());
    rclcpp::shutdown();
}
