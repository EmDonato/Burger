#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include "stm32_nucleo_f303re_driver/srv/cmd.hpp"

class JoyServiceNode : public rclcpp::Node {
public:
    JoyServiceNode() : Node("joy_service_node") {
        this->declare_parameter("arm_button", 0);
        this->declare_parameter("reset_button", 6);
        this->declare_parameter("joy_service", true); 

        update_parameters();

        client_ = this->create_client<stm32_nucleo_f303re_driver::srv::Cmd>("/cmd");

        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&JoyServiceNode::joy_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Joy Service Node Robust Version Started.");
        RCLCPP_INFO(this->get_logger(), "Monitoring Button: %d and %d", arm_button_, reset_button_);
    }

private:
    void update_parameters() {
        arm_button_ = this->get_parameter("arm_button").as_int();
        reset_button_ = this->get_parameter("reset_button").as_int();
        joy_service_ = this->get_parameter("joy_service").as_bool();
    }

    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        update_parameters();

        if (arm_button_ < 0 || arm_button_ >= static_cast<int>(msg->buttons.size())) {
            return;
        }

        int current_state  = msg->buttons[arm_button_];
        int reset_request_ = msg->buttons[reset_button_];
        auto now = this->now();

        if (current_state == 1 && last_button_state_ == 0) {
            double elapsed = (now - last_call_time_).seconds();
            
            if (elapsed > 0.5) { 
                if (joy_service_) {
                    call_arm_service();
                    last_call_time_ = now;
                } else {
                    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                        "Service call blocked: joy_service is FALSE");
                }
            }
        }
        if (reset_request_ == 1) {
                    call_reset_service();
                } 
             
        
        last_button_state_ = current_state;
    }

    void call_arm_service() {
        if (!client_->service_is_ready()) {
            RCLCPP_ERROR(this->get_logger(), "Service /cmd NOT READY. Is the driver running?");
            return;
        }

        auto request = std::make_shared<stm32_nucleo_f303re_driver::srv::Cmd::Request>();
        request->command = 1;

        RCLCPP_INFO(this->get_logger(), ">> Sending ARM Toggle Request...");
        
        client_->async_send_request(request, 
            [this](rclcpp::Client<stm32_nucleo_f303re_driver::srv::Cmd>::SharedFuture future) {
                try {
                    auto response = future.get();
                    if (response->success) {
                        RCLCPP_INFO(this->get_logger(), "<< ARM Request Accepted by Driver");
                    } else {
                        RCLCPP_ERROR(this->get_logger(), "<< ARM Request Rejected!");
                    }
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Service Exception: %s", e.what());
                }
            });
    }

    void call_reset_service() {
        if (!client_->service_is_ready()) {
            RCLCPP_ERROR(this->get_logger(), "Service /cmd NOT READY. Is the driver running?");
            return;
        }

        auto request = std::make_shared<stm32_nucleo_f303re_driver::srv::Cmd::Request>();
        request->command = 2;

        RCLCPP_INFO(this->get_logger(), ">> Sending RESET Request...");
        
        client_->async_send_request(request, 
            [this](rclcpp::Client<stm32_nucleo_f303re_driver::srv::Cmd>::SharedFuture future) {
                try {
                    auto response = future.get();
                    if (response->success) {
                        RCLCPP_INFO(this->get_logger(), "<< RESET Request Accepted by Driver");
                    } else {
                        RCLCPP_ERROR(this->get_logger(), "<< RESET Request Rejected!");
                    }
                } catch (const std::exception &e) {
                    RCLCPP_ERROR(this->get_logger(), "Service Exception: %s", e.what());
                }
            });
    }
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    rclcpp::Client<stm32_nucleo_f303re_driver::srv::Cmd>::SharedPtr client_;
    
    int arm_button_;
    int reset_button_;
    bool joy_service_;
    int last_button_state_ = 0;
    rclcpp::Time last_call_time_{0, 0, RCL_ROS_TIME};
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<JoyServiceNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}