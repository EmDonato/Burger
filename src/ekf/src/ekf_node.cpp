#include "rclcpp/rclcpp.hpp"

#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"

#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"

#include <Eigen/Dense>

#include <array>
#include <cmath>
#include <chrono>
#include <memory>
#include <mutex>

class Ekf : public rclcpp::Node
{
public:
    Ekf()
    : Node("ekf"),
      verbal_logger_(this->get_logger().get_child("verbal_logger"))
    {
        this->declare_parameter<double>("wheel_base", 0.2);
        this->declare_parameter<double>("radius", 0.0346);

        this->declare_parameter<double>("alpha_v", 6.0);
        this->declare_parameter<double>("alpha_w", 10.0);

        this->declare_parameter<double>("nominal_dt", 0.02);
        this->declare_parameter<double>("max_dt", 0.1);

        this->declare_parameter<double>("imu_mahalanobis_threshold", 9.0);
        this->declare_parameter<double>("enc_mahalanobis_threshold", 9.21);

        this->get_parameter("radius", radius_);
        this->get_parameter("wheel_base", wheel_base_);
        this->get_parameter("alpha_v", alpha_v_);
        this->get_parameter("alpha_w", alpha_w_);
        this->get_parameter("nominal_dt", nominal_dt_);
        this->get_parameter("max_dt", max_dt_);
        this->get_parameter("imu_mahalanobis_threshold", imu_gate_);
        this->get_parameter("enc_mahalanobis_threshold", enc_gate_);

        x_.setZero();

        P_.setZero();
        P_.diagonal() << 0.0001,
                         0.0001,
                         0.01,
                         0.01,
                         0.1,
                         0.01;

        Q_.setZero();
        Q_(3, 3) = 0.0025;
        Q_(4, 4) = 0.0225;
        Q_(5, 5) = 0.000025;

        F_.setIdentity();

        H_enc_ <<
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0;

        R_enc_.setZero();
        R_enc_.diagonal() << 0.0004, 0.0025;

        H_imu_ << 0.0, 0.0, 0.0, 0.0, 1.0, 1.0;

        R_imu_.setZero();
        R_imu_(0, 0) = 0.0009;

        last_predict_time_ = this->now();

        tf_broadcaster_ =
            std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        odom_pub_ =
            this->create_publisher<nav_msgs::msg::Odometry>("/odom", 10);

        ref_sub_ =
            this->create_subscription<geometry_msgs::msg::Twist>(
                "/cmd_vel",
                rclcpp::QoS(10),
                std::bind(&Ekf::reference_reader, this, std::placeholders::_1)
            );

        imu_sub_ =
            this->create_subscription<sensor_msgs::msg::Imu>(
                "imu",
                rclcpp::SensorDataQoS(),
                std::bind(&Ekf::imu_correction, this, std::placeholders::_1)
            );

        enc_sub_ =
            this->create_subscription<geometry_msgs::msg::TwistStamped>(
                "enc/twist_meas",
                rclcpp::SensorDataQoS(),
                std::bind(&Ekf::encoder_correction, this, std::placeholders::_1)
            );

        timer_ =
            this->create_wall_timer(
                std::chrono::milliseconds(20),
                std::bind(&Ekf::predict, this)
            );

        RCLCPP_INFO(this->get_logger(), "EKF node started");
    }

private:
    using Vector6d = Eigen::Matrix<double, 6, 1>;
    using Matrix6d = Eigen::Matrix<double, 6, 6>;
    using Matrix2d = Eigen::Matrix<double, 2, 2>;
    using Matrix1d = Eigen::Matrix<double, 1, 1>;

    rclcpp::Logger verbal_logger_;
    std::mutex ekf_mutex_;

    double radius_ = 0.0346;
    double wheel_base_ = 0.2;

    double alpha_v_ = 6.0;
    double alpha_w_ = 10.0;

    double nominal_dt_ = 0.02;
    double max_dt_ = 0.1;

    double imu_gate_ = 9.0;
    double enc_gate_ = 9.21;

    double v_ref_ = 0.0;
    double w_ref_ = 0.0;

    rclcpp::Time last_predict_time_;

    /*
        State:
        x_(0) = x
        x_(1) = y
        x_(2) = theta
        x_(3) = v
        x_(4) = w
        x_(5) = gyro bias
    */
    Vector6d x_;

    Matrix6d P_;
    Matrix6d Q_;
    Matrix6d F_;

    Eigen::Matrix<double, 2, 6> H_enc_;
    Matrix2d R_enc_;
    Matrix2d S_enc_;
    Eigen::Matrix<double, 2, 1> diff_enc_;
    Eigen::Matrix<double, 6, 2> K_enc_;

    Eigen::Matrix<double, 1, 6> H_imu_;
    Matrix1d R_imu_;
    Matrix1d S_imu_;
    Matrix1d diff_imu_;
    Eigen::Matrix<double, 6, 1> K_imu_;

    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr ref_sub_;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr enc_sub_;

    rclcpp::TimerBase::SharedPtr timer_;

    void predict()
    {
        std::lock_guard<std::mutex> lock(ekf_mutex_);

        const rclcpp::Time now_time = this->now();

        predict_to_time(now_time);

        publish_odometry(now_time);
    }

    void predict_to_time(const rclcpp::Time& target_time)
    {
        double dt = (target_time - last_predict_time_).seconds();

        if (dt <= 0.0) {
            return;
        }

        if (dt > max_dt_) {
            last_predict_time_ = target_time;
            return;
        }

        last_predict_time_ = target_time;

        const double theta = x_(2);
        const double v = x_(3);
        const double w = x_(4);

        const double c = std::cos(theta);
        const double s = std::sin(theta);

        x_(0) += v * c * dt;
        x_(1) += v * s * dt;
        x_(2) += w * dt;

        x_(3) += alpha_v_ * (v_ref_ - x_(3)) * dt;
        x_(4) += alpha_w_ * (w_ref_ - x_(4)) * dt;

        normalize_angle(x_(2));

        F_.setIdentity();

        F_(0, 2) = -v * s * dt;
        F_(0, 3) =  c * dt;

        F_(1, 2) =  v * c * dt;
        F_(1, 3) =  s * dt;

        F_(2, 4) = dt;

        F_(3, 3) = 1.0 - alpha_v_ * dt;
        F_(4, 4) = 1.0 - alpha_w_ * dt;

        const double q_scale = dt / nominal_dt_;

        P_ = F_ * P_ * F_.transpose() + q_scale * Q_;

        symmetrize_covariance();
    }

    void reference_reader(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        std::lock_guard<std::mutex> lock(ekf_mutex_);

        v_ref_ = msg->linear.x;
        w_ref_ = msg->angular.z;
    }

    void imu_correction(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        std::lock_guard<std::mutex> lock(ekf_mutex_);

        const rclcpp::Time stamp = get_valid_stamp(msg->header.stamp);
        predict_to_time(stamp);

        Matrix1d z;
        z(0, 0) = msg->angular_velocity.z;

        const double imu_cov_z = msg->angular_velocity_covariance[8];

        if (imu_cov_z > 0.0) {
            R_imu_(0, 0) = imu_cov_z;
        }

        diff_imu_ = z - H_imu_ * x_;

        S_imu_ =
            H_imu_ *
            P_ *
            H_imu_.transpose()
            +
            R_imu_;

        const double maha =
            diff_imu_(0, 0) *
            (1.0 / S_imu_(0, 0)) *
            diff_imu_(0, 0);

        if (maha > imu_gate_) {
            RCLCPP_WARN_THROTTLE(
                verbal_logger_,
                *this->get_clock(),
                1000,
                "IMU rejected by Mahalanobis gate: %.3f",
                maha
            );
            publish_odometry(stamp);
            return;
        }

        K_imu_ =
            P_ *
            H_imu_.transpose() *
            S_imu_.inverse();

        x_ = x_ + K_imu_ * diff_imu_(0, 0);

        normalize_angle(x_(2));

        const Matrix6d I = Matrix6d::Identity();

        P_ =
            (I - K_imu_ * H_imu_) *
            P_ *
            (I - K_imu_ * H_imu_).transpose()
            +
            K_imu_ *
            R_imu_ *
            K_imu_.transpose();

        symmetrize_covariance();

        publish_odometry(stamp);
    }

    void encoder_correction(const geometry_msgs::msg::TwistStamped::SharedPtr msg)
    {
        std::lock_guard<std::mutex> lock(ekf_mutex_);

        const rclcpp::Time stamp = get_valid_stamp(msg->header.stamp);
        predict_to_time(stamp);

        Eigen::Matrix<double, 2, 1> z;

        z <<
            msg->twist.linear.x,
            msg->twist.angular.z;

        diff_enc_ = z - H_enc_ * x_;

        S_enc_ =
            H_enc_ *
            P_ *
            H_enc_.transpose()
            +
            R_enc_;

        const double maha =
            (diff_enc_.transpose() *
             S_enc_.inverse() *
             diff_enc_)(0, 0);

        if (maha > enc_gate_) {
            RCLCPP_WARN_THROTTLE(
                verbal_logger_,
                *this->get_clock(),
                1000,
                "Encoder rejected by Mahalanobis gate: %.3f",
                maha
            );
            publish_odometry(stamp);
            return;
        }

        K_enc_ =
            P_ *
            H_enc_.transpose() *
            S_enc_.inverse();

        x_ = x_ + K_enc_ * diff_enc_;

        normalize_angle(x_(2));

        const Matrix6d I = Matrix6d::Identity();

        P_ =
            (I - K_enc_ * H_enc_) *
            P_ *
            (I - K_enc_ * H_enc_).transpose()
            +
            K_enc_ *
            R_enc_ *
            K_enc_.transpose();

        symmetrize_covariance();

        publish_odometry(stamp);
    }

    void publish_odometry(const rclcpp::Time& stamp)
    {
        nav_msgs::msg::Odometry odom;

        odom.header.stamp = stamp;
        odom.header.frame_id = "odom";
        odom.child_frame_id = "base_link";

        odom.pose.pose.position.x = x_(0);
        odom.pose.pose.position.y = x_(1);
        odom.pose.pose.position.z = 0.0;

        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, x_(2));
        q.normalize();

        odom.pose.pose.orientation.x = q.x();
        odom.pose.pose.orientation.y = q.y();
        odom.pose.pose.orientation.z = q.z();
        odom.pose.pose.orientation.w = q.w();

        odom.twist.twist.linear.x = x_(3);
        odom.twist.twist.angular.z = x_(4);

        fill_odometry_covariance(odom);

        odom_pub_->publish(odom);

        geometry_msgs::msg::TransformStamped tf_msg;

        tf_msg.header.stamp = stamp;
        tf_msg.header.frame_id = "odom";
        tf_msg.child_frame_id = "base_link";

        tf_msg.transform.translation.x = x_(0);
        tf_msg.transform.translation.y = x_(1);
        tf_msg.transform.translation.z = 0.0;

        tf_msg.transform.rotation.x = q.x();
        tf_msg.transform.rotation.y = q.y();
        tf_msg.transform.rotation.z = q.z();
        tf_msg.transform.rotation.w = q.w();

        tf_broadcaster_->sendTransform(tf_msg);
    }

    void fill_odometry_covariance(nav_msgs::msg::Odometry& odom)
    {
        odom.pose.covariance.fill(0.0);
        odom.twist.covariance.fill(0.0);

        odom.pose.covariance[0]  = P_(0, 0);
        odom.pose.covariance[1]  = P_(0, 1);
        odom.pose.covariance[5]  = P_(0, 2);

        odom.pose.covariance[6]  = P_(1, 0);
        odom.pose.covariance[7]  = P_(1, 1);
        odom.pose.covariance[11] = P_(1, 2);

        odom.pose.covariance[30] = P_(2, 0);
        odom.pose.covariance[31] = P_(2, 1);
        odom.pose.covariance[35] = P_(2, 2);

        odom.pose.covariance[14] = 1e6;
        odom.pose.covariance[21] = 1e6;
        odom.pose.covariance[28] = 1e6;

        odom.twist.covariance[0]  = P_(3, 3);
        odom.twist.covariance[5]  = P_(3, 4);

        odom.twist.covariance[30] = P_(4, 3);
        odom.twist.covariance[35] = P_(4, 4);

        odom.twist.covariance[7]  = 1e6;
        odom.twist.covariance[14] = 1e6;
        odom.twist.covariance[21] = 1e6;
        odom.twist.covariance[28] = 1e6;
    }

    rclcpp::Time get_valid_stamp(const builtin_interfaces::msg::Time& stamp_msg)
    {
        rclcpp::Time stamp(stamp_msg);

        if (stamp.nanoseconds() == 0) {
            return this->now();
        }

        return stamp;
    }

    void symmetrize_covariance()
    {
        P_ = 0.5 * (P_ + P_.transpose());
    }

    void normalize_angle(double& angle)
    {
        while (angle > M_PI) {
            angle -= 2.0 * M_PI;
        }

        while (angle < -M_PI) {
            angle += 2.0 * M_PI;
        }
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<Ekf>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}