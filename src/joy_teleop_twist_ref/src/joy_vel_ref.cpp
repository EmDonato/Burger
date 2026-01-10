#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <algorithm>

class JoyVelRef : public rclcpp::Node {
public:
    JoyVelRef() : Node("joy_vel_ref_node") {
        
        this->declare_parameter("max_vel", 0.463);
        this->declare_parameter("scales", 0.8);
        this->declare_parameter("wheel_base", 0.2); 

        max_vel_ = this->get_parameter("max_vel").as_double();
        scales_ = this->get_parameter("scales").as_double();
        wheel_base_ = this->get_parameter("wheel_base").as_double();

        pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", rclcpp::SensorDataQoS());
    
        sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/joy/cmd_vel", 
            rclcpp::SensorDataQoS(), 
            std::bind(&JoyVelRef::send_vel_twist_, this, std::placeholders::_1)
        );
        RCLCPP_INFO(get_logger(), "JOY ref vel started");

    }

private:
    double max_vel_;
    double scales_;
    double wheel_base_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;

    void send_vel_twist_(const geometry_msgs::msg::Twist::SharedPtr msg) {
        auto msg_vel = geometry_msgs::msg::Twist();
        
        float vl, vr;
        float aus_l, aus_r;

        if (msg->angular.z < 0) {
            aus_l = 1.0;
            aus_r = 1.0 - std::abs(msg->angular.z * 2.0);
        } else {
            aus_l = 1.0 - std::abs(msg->angular.z * 2.0);
            aus_r = 1.0;
        }

        float limit = max_vel_ * scales_;
        vl = std::clamp(static_cast<float>(max_vel_ * scales_ * msg->linear.x * aus_l), -limit, limit);
        vr = std::clamp(static_cast<float>(max_vel_ * scales_ * msg->linear.x * aus_r), -limit, limit);

        float v = (vr + vl) / 2.0f;
        float w = (vr - vl) / static_cast<float>(wheel_base_);

        msg_vel.linear.x = v;
        msg_vel.angular.z = w;

        pub_->publish(msg_vel);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyVelRef>());
    rclcpp::shutdown();
    return 0;
}