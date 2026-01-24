#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "stm32_nucleo_f303re_driver/msg/wheelspeed.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "sensor_msgs/msg/magnetic_field.hpp"
#include "std_msgs/msg/bool.hpp"
#include "stm32_nucleo_f303re_driver/srv/cmd.hpp"

#include <boost/circular_buffer.hpp>
#include <chrono>
#include <cstring>
#include <vector>

#include "parsing_types.hpp"
#include <tf2/LinearMath/Quaternion.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

class StmDriver : public rclcpp::Node
{
public:
    StmDriver() : Node("stm32_driver"),verbal_logger(this->get_logger().get_child("verbal_logger"))
    {
        // Parameter declaration
        this->declare_parameter("port", "/dev/ttyACM0");
        this->declare_parameter("baudrate", 115200);

        this->get_parameter("port", port_);
        this->get_parameter("baudrate", baudrate_);

        try {
            open_serial_();
        } catch (const std::exception &e) {
            RCLCPP_FATAL(this->get_logger(), "Could not open serial port: %s", e.what());
            rclcpp::shutdown();
        }

        // Publishers initialization
        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("imu", rclcpp::SensorDataQoS());
        enc_pub_ = create_publisher<geometry_msgs::msg::Twist>("enc/twist", rclcpp::SensorDataQoS());
        wheels_pub_ = create_publisher<stm32_nucleo_f303re_driver::msg::Wheelspeed>("enc/twist_wheels", rclcpp::SensorDataQoS());
        mag_pub_ = create_publisher<sensor_msgs::msg::MagneticField>("magnetometer", rclcpp::SensorDataQoS());
        
        // Transient Local QoS ensures the last state is available to new subscribers
        arm_pub_ = create_publisher<std_msgs::msg::Bool>(
            "robot_status/is_armed", rclcpp::QoS(rclcpp::KeepLast(1)).transient_local());

        // Subscriptions
        ref_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 
            rclcpp::SensorDataQoS(), 
            std::bind(&StmDriver::send_vel_twist_, this, std::placeholders::_1)
        );    

        // Services
        srv_ = this->create_service<stm32_nucleo_f303re_driver::srv::Cmd>(
            "cmd",
            std::bind(&StmDriver::handle_service, this, std::placeholders::_1, std::placeholders::_2));

        // Main Timer Loop (200Hz)
        timer_ = create_wall_timer(
            std::chrono::milliseconds(5),
            std::bind(&StmDriver::read_serial_, this));
        send_command(static_cast<uint8_t>(RESET_ID));
        RCLCPP_INFO(this->get_logger(), "STM32 Driver started on %s @ %d baud", port_.c_str(), baudrate_);
    }

    ~StmDriver()
    {
        if (serial_fd_ >= 0) {
            close(serial_fd_);
            RCLCPP_INFO(this->get_logger(), "Serial port closed.");
        }
    }

private:
    rclcpp::Logger verbal_logger;
    std::string port_;
    int baudrate_;
    int serial_fd_{-1};
    pollfd pfd_{};
    uint8_t tmp_buf_[128];
    boost::circular_buffer<uint8_t> rx_buffer_{512};
    float radius = 0.0346f ;
    // Parser State Machine
    ParseState state_{ParseState::WAIT_HEADER1};
    uint8_t msg_type_{0};
    uint8_t msg_len_{0};
    uint8_t checksum_{0};
    std::vector<uint8_t> payload_;

    // ROS 2 Interfaces
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<sensor_msgs::msg::MagneticField>::SharedPtr mag_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr enc_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr arm_pub_;
    rclcpp::Publisher<stm32_nucleo_f303re_driver::msg::Wheelspeed>::SharedPtr wheels_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr ref_sub_;
    rclcpp::Service<stm32_nucleo_f303re_driver::srv::Cmd>::SharedPtr srv_;
    rclcpp::TimerBase::SharedPtr timer_;

    bool last_arm_state_ = true;

    void open_serial_()
    {
        serial_fd_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (serial_fd_ < 0) {
            throw std::runtime_error("Failed to open port");
        }

        termios tty{};
        tcgetattr(serial_fd_, &tty);
        
        // Setting baudrate
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);

        // 8N1 Mode
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
        
        // Raw input/output
        tty.c_lflag = 0;
        tty.c_iflag = 0;
        tty.c_oflag = 0;
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 0;

        tcsetattr(serial_fd_, TCSANOW, &tty);
        
        // Flush port to clear noise/garbage
        tcflush(serial_fd_, TCIOFLUSH);

        pfd_.fd = serial_fd_;
        pfd_.events = POLLIN;
    }

    void read_serial_()
    {
        // Poll for new data with 0ms timeout (non-blocking)
        while (poll(&pfd_, 1, 0) > 0) {
            ssize_t n = read(serial_fd_, tmp_buf_, sizeof(tmp_buf_));
            if (n > 0) {
                for (ssize_t i = 0; i < n; ++i)
                    rx_buffer_.push_back(tmp_buf_[i]);
            }
        }
        parse_();
    }

    void parse_()
    {
        while (!rx_buffer_.empty()) {
            uint8_t c = rx_buffer_.front();
            rx_buffer_.pop_front();

            switch (state_) {
                case ParseState::WAIT_HEADER1:
                    if (c == HDR1) {
                        checksum_ = c; // Start checksum calculation
                        state_ = ParseState::WAIT_HEADER2;
                    }
                    break;
                case ParseState::WAIT_HEADER2:
                    if (c == HDR2) {
                        checksum_ ^= c;
                        state_ = ParseState::WAIT_TYPE;
                    } else {
                        state_ = ParseState::WAIT_HEADER1;
                    }
                    break;
                case ParseState::WAIT_TYPE:
                    msg_type_ = c;
                    checksum_ ^= c;
                    state_ = ParseState::WAIT_LEN;
                    break;
                case ParseState::WAIT_LEN:
                    msg_len_ = c;
                    payload_.clear();
                    checksum_ ^= c;
                    if (msg_len_ == 0) state_ = ParseState::WAIT_CHECKSUM;
                    else state_ = ParseState::WAIT_PAYLOAD;
                    break;
                case ParseState::WAIT_PAYLOAD:
                    payload_.push_back(c);
                    checksum_ ^= c;
                    if (payload_.size() >= msg_len_)
                        state_ = ParseState::WAIT_CHECKSUM;
                    break;
                case ParseState::WAIT_CHECKSUM:
                    if (checksum_ == c) {
                        handle_message_();
                    } else {
                        RCLCPP_WARN(this->get_logger(), "CRC Error. Type: 0x%02X, Calc: 0x%02X, Recv: 0x%02X", 
                                    msg_type_, checksum_, c);
                    }
                    state_ = ParseState::WAIT_HEADER1;
                    break;
            }
        }
    }

    void handle_message_()
    {
        switch (msg_type_)
        {
            case IMU_ID:
            {
                // Format: uint32_t timestamp + 15 floats (3 Euler, 3 Accel, 3 Gyro, 3 Mag, 3 LinearAccel)
                constexpr size_t EXPECTED_LEN = 4 + (15 * sizeof(float));
                if (payload_.size() != EXPECTED_LEN) return;

                float d[15];
                std::memcpy(d, payload_.data() + 4, 15 * sizeof(float));

                auto imu = sensor_msgs::msg::Imu();
                imu.header.stamp = this->now();
                imu.header.frame_id = "imu_link";

                tf2::Quaternion q;
                q.setRPY(d[0], d[1], d[2]);
                imu.orientation.x = q.x(); imu.orientation.y = q.y();
                imu.orientation.z = q.z(); imu.orientation.w = q.w();
                
                imu.linear_acceleration.x = d[3];
                imu.linear_acceleration.y = d[4];
                imu.linear_acceleration.z = d[5];
                
                imu.angular_velocity.x = d[6];
                imu.angular_velocity.y = d[7];
                imu.angular_velocity.z = d[8];
                imu_pub_->publish(imu);

                auto mag = sensor_msgs::msg::MagneticField();
                mag.header.stamp = imu.header.stamp;
                mag.header.frame_id = "mag_link";
                mag.magnetic_field.x = d[9];
                mag.magnetic_field.y = d[10];
                mag.magnetic_field.z = d[11];
                mag_pub_->publish(mag);
                break;
            }

            case ENC_ID:
            {    

                if (payload_.size() < 2 * sizeof(float)) return;
                float d[2];
                std::memcpy(d, payload_.data(), 2 * sizeof(float));
                float d_custom[4];
                std::memcpy(d_custom, payload_.data() + (2 * sizeof(float)), 4 * sizeof(float));
                auto custom_msg = stm32_nucleo_f303re_driver::msg::Wheelspeed();
                auto twist = geometry_msgs::msg::Twist();
                custom_msg.header.stamp = this->now();
                custom_msg.header.frame_id = "";
                twist.linear.x = d[0];
                twist.angular.z = d[1];
                
                custom_msg.speed[0] = d_custom[0]*2*M_PI*radius/60;
                custom_msg.speed[1] = d_custom[1]*2*M_PI*radius/60;
                custom_msg.pwm[2] = d_custom[2];
                custom_msg.pwm[3] = d_custom[3];
                enc_pub_->publish(twist);
                wheels_pub_->publish(custom_msg);

                break;
            }

            case HB_ID:
            {
                if (payload_.empty()) return;
                uint32_t d[2];
                std::memcpy(d, payload_.data(), 2 * sizeof(float));
                bool current_arm_state = (d[1] == (uint32_t)1);
                
                if (current_arm_state != last_arm_state_) {
                    RCLCPP_INFO(this->get_logger(), "Robot Hardware Status: %s", 
                                current_arm_state ? "ARMED" : "DISARMED");
                }
                last_arm_state_ = current_arm_state;
                auto msg = std_msgs::msg::Bool();
                msg.data = current_arm_state;
                arm_pub_->publish(msg);
                break;
            }
            case STRING_ID:
            {
                if (payload_.empty()) return;

                std::string msg(
                    reinterpret_cast<const char*>(payload_.data()),
                    payload_.size()
                );

                RCLCPP_INFO(verbal_logger, "%s", msg.c_str());
                break;
            }

        }
    }

    void send_vel_twist_(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        constexpr uint8_t LEN = sizeof(float) * 2;
        float payload[2];
        payload[0] = static_cast<float>(msg->linear.x);
        payload[1] = static_cast<float>(msg->angular.z);

        uint8_t checksum = HDR1 ^ HDR2 ^ LEN ^ CMD_VEL_ID;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(payload);
        for (uint8_t i = 0; i < LEN; ++i) checksum ^= p[i];

        std::vector<uint8_t> packet = {HDR1, HDR2, LEN, CMD_VEL_ID};
        packet.insert(packet.end(), p, p + LEN);
        packet.push_back(checksum);

        write(serial_fd_, packet.data(), packet.size());
    }
    void send_command(uint8_t command)
    {
        uint8_t hdr1 = 0xAA;
        uint8_t hdr2 = 0x55;
        uint8_t len = 0;      
        uint8_t type = command;

        uint8_t checksum = hdr1;
        checksum ^= hdr2;
        checksum ^= len;    
        checksum ^= type;

        
        uint8_t packet[5] = {hdr1, hdr2, len, type, checksum};

        tcflush(serial_fd_, TCOFLUSH); 
        write(serial_fd_, packet, 5);
        
        RCLCPP_INFO(this->get_logger(), "Sent ARM command to STM32. CS: 0x%02X", checksum);
    }
    void handle_service(
        const std::shared_ptr<stm32_nucleo_f303re_driver::srv::Cmd::Request> req,
        std::shared_ptr<stm32_nucleo_f303re_driver::srv::Cmd::Response> res)
    {
        // Command 1 = Arm, Command 0 = Disarm //command 2 = reset
        if (req->command == 1 || req->command == 0) {
            send_command(static_cast<uint8_t>(ARM_ID));
            res->success = true;
        }else if(req->command == 2){
            send_command(static_cast<uint8_t>(RESET_ID));
            res->success = true;    
            RCLCPP_INFO(this->get_logger(), "Reset sent");
        }
         else {
            RCLCPP_WARN(this->get_logger(), "Invalid Service Command: %d", req->command);
            res->success = false;
        }
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<StmDriver>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}