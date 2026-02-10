#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <algorithm>
#include <cmath>

class JoyVelRef : public rclcpp::Node {
public:
    JoyVelRef() : Node("joy_vel_ref_node") {

        // ===============================
        // Parameters
        // ===============================
        this->declare_parameter("max_vel", 0.30);     // [m/s]
        this->declare_parameter("scales", 1.0);        // [0..1]
        this->declare_parameter("wheel_base", 0.2);    // [m]

        max_vel_    = this->get_parameter("max_vel").as_double();
        scales_     = this->get_parameter("scales").as_double();
        wheel_base_ = this->get_parameter("wheel_base").as_double();

        // ===============================
        // ROS interfaces
        // ===============================
        pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "cmd_vel",
            rclcpp::SensorDataQoS()
        );

        sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/joy/cmd_vel",
            rclcpp::SensorDataQoS(),
            std::bind(&JoyVelRef::send_vel_twist_, this, std::placeholders::_1)
        );

        RCLCPP_INFO(get_logger(), "JOY ref vel started (symmetric normalized mixing)");
    }

private:
    double max_vel_;
    double scales_;
    double wheel_base_;

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;

    void send_vel_twist_(const geometry_msgs::msg::Twist::SharedPtr msg) {

        // ===============================
        // 1) Joystick -> reference signals
        // ===============================
        float v_ref = static_cast<float>(max_vel_ * scales_ * msg->linear.x);
        float w_mix = static_cast<float>(max_vel_ * scales_ * msg->angular.z);

        // ===============================
        // 2) Symmetric wheel mixing
        // ===============================
        float v_l = v_ref - w_mix;
        float v_r = v_ref + w_mix;

        // ===============================
        // 3) Normalization (respect limits)
        // ===============================
        float limit = static_cast<float>(max_vel_ * scales_);
        float max_abs = std::max(std::abs(v_l), std::abs(v_r));

        if (max_abs > limit) {
            float scale = limit / max_abs;
            v_l *= scale;
            v_r *= scale;
        }

        // ===============================
        // 4) Reconstruct Twist (physical)
        // ===============================
        geometry_msgs::msg::Twist out;

        out.linear.x  = (v_r + v_l) * 0.5f;
        out.angular.z = (v_r - v_l) / static_cast<float>(wheel_base_);

        pub_->publish(out);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyVelRef>());
    rclcpp::shutdown();
    return 0;
}
