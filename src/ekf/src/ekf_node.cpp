#include "rclcpp/rclcpp.hpp"

#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>

#include <chrono>

class ekf : public rclcpp::Node
{
public:
    ekf() : Node("ekf"),verbal_logger(this->get_logger().get_child("verbal_logger"))
    {

        this->declare_parameter("wheel_base", 0.2f);
        this->declare_parameter("radius", 0.0346f );

        this->get_parameter("radius", radius);
        this->get_parameter("wheel_base", wheel_base);

        // Publishers initialization

        tf_broadcaster_ =
        std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        odom_pub_ =
        this->create_publisher<nav_msgs::msg::Odometry>(
            "/odom", 10);

        // Subscriptions
        ref_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 
            rclcpp::QoS(10), 
            std::bind(&ekf::reference_reader, this, std::placeholders::_1)
        );    
        imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>("imu",
            rclcpp::SensorDataQoS(),
            std::bind(&ekf::imu_correction, this, std::placeholders::_1)
        );
        enc_sub_ = this->create_subscription<geometry_msgs::msg::TwistStamped>("enc/twist_meas", 
            rclcpp::SensorDataQoS(),
            std::bind(&ekf::encoder_correction, this, std::placeholders::_1)
        );


        // Main Timer Loop (50Hz)
        timer_ = create_wall_timer(
            std::chrono::milliseconds(20),
            std::bind(&ekf::predict, this)); // sent in the predict

        RCLCPP_INFO(this->get_logger(), "ekf node started with encoders and imu");
    }

    ~ekf()
    {

    }

private:
    rclcpp::Logger verbal_logger;
    uint8_t tmp_buf_[128];
    rclcpp::Time last_predict_time_;

    //reference
    float v_long_ref = 0.0;
    float w_z_ref = 0.0;

    //geometry chassis
    float radius;
    float wheel_base;

    //state
    float x = 0.0f;
    float y = 0.0f;
    float theta = 0.0f;
    float v = 0.0f;
    float w = 0.0f;
    float bias = 0.0f;

    // ROS 2 Interfaces
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr ref_sub_;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr enc_sub_;

    rclcpp::TimerBase::SharedPtr timer_;


};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ekf>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}