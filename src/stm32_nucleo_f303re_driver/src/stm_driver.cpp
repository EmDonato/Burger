#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/magnetic_field.hpp"
#include <boost/circular_buffer.hpp>
#include <chrono>
#include <cstring>
#include <vector>

#include <tf2/LinearMath/Quaternion.h>

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

// =====================
// PROTOCOL CONSTANTS
// =====================
static constexpr uint8_t HDR1 = 0xAA;
static constexpr uint8_t HDR2 = 0x55;

enum MsgID : uint8_t {
    IMU_ID     = 0x01,
    ENC_ID     = 0x02,
    CMD_VEL_ID = 0x03,
    ARM_ID     = 0x04,
    HB_ID      = 0x05,
    STRING_ID  = 0x06
};

enum class ParseState {
    WAIT_HEADER1,
    WAIT_HEADER2,
    WAIT_TYPE,
    WAIT_LEN,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM
};

class StmDriver : public rclcpp::Node
{
public:
    StmDriver() : Node("stm32_driver")
    {
        declare_parameter("port", "/dev/ttyACM0");
        declare_parameter("baudrate", 115200);

        get_parameter("port", port_);
        get_parameter("baudrate", baudrate_);

        open_serial_();

        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>(
            "imu", rclcpp::SensorDataQoS());
        enc_pub_ = create_publisher<geometry_msgs::msg::Twist>(
            "enc/twist", rclcpp::SensorDataQoS());
        mag_pub_ = create_publisher<sensor_msgs::msg::MagneticField>(
            "magnetometer", rclcpp::SensorDataQoS());

        timer_ = create_wall_timer(
            std::chrono::milliseconds(5),
            std::bind(&StmDriver::read_serial_, this));

        RCLCPP_INFO(get_logger(), "STM32 IMU driver started");
    }

    ~StmDriver()
    {
        if (serial_fd_ >= 0)
            close(serial_fd_);
    }

private:
    // =====================
    // SERIAL
    // =====================
    std::string port_;
    int baudrate_;
    int serial_fd_{-1};
    pollfd pfd_{};

    uint8_t tmp_buf_[128];
    boost::circular_buffer<uint8_t> rx_buffer_{512};

    // =====================
    // PARSER
    // =====================
    ParseState state_{ParseState::WAIT_HEADER1};
    uint8_t msg_type_{0};
    uint8_t msg_len_{0};
    uint8_t checksum_{0};
    std::vector<uint8_t> payload_;

    // =====================
    // ROS
    // =====================
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<sensor_msgs::msg::MagneticField>::SharedPtr mag_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr enc_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    // =====================
    // SERIAL SETUP
    // =====================
    void open_serial_()
    {
        serial_fd_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (serial_fd_ < 0)
            throw std::runtime_error("Failed to open serial port");

        termios tty{};
        tcgetattr(serial_fd_, &tty);

        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);

        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);

        tty.c_lflag = 0;
        tty.c_iflag = 0;
        tty.c_oflag = 0;
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 0;

        tcsetattr(serial_fd_, TCSANOW, &tty);

        pfd_.fd = serial_fd_;
        pfd_.events = POLLIN;
    }

    // =====================
    // READ SERIAL
    // =====================
    void read_serial_()
    {
        while (poll(&pfd_, 1, 0) > 0) {
            ssize_t n = read(serial_fd_, tmp_buf_, sizeof(tmp_buf_));
            if (n > 0) {
                for (ssize_t i = 0; i < n; ++i)
                    rx_buffer_.push_back(tmp_buf_[i]);
            }
        }
        parse_();
    }

    // =====================
    // PARSER FSM
    // =====================
    void parse_()
    {
        while (!rx_buffer_.empty()) {
            uint8_t c = rx_buffer_.front();
            rx_buffer_.pop_front();

            switch (state_) {

            case ParseState::WAIT_HEADER1:
                if (c == HDR1) {
                    checksum_ = c;
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
                state_ = ParseState::WAIT_PAYLOAD;
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
                    RCLCPP_ERROR(get_logger(),
                        "Checksum error (type=0x%02X)", msg_type_);
                }
                state_ = ParseState::WAIT_HEADER1;
                break;
            }
        }
    }

    // =====================
    // MESSAGE HANDLER
    // =====================
    void handle_message_()
    {



        switch (msg_type_)
        {
        case IMU_ID :
        {
            constexpr size_t EXPECTED_LEN =
                sizeof(uint32_t) + 15 * sizeof(float);

            if (payload_.size() != EXPECTED_LEN)
                return;

            uint32_t id;
            float d[15];

            std::memcpy(&id, payload_.data(), 4);
            std::memcpy(d, payload_.data() + 4, 15 * sizeof(float));

            float roll = d[0], pitch = d[1], yaw = d[2];
            float ax = d[3], ay = d[4], az = d[5];
            float gx = d[6], gy = d[7], gz = d[8];
            float mx = d[9], my = d[10], mz = d[11];
            float bx = d[12], by = d[13], bz = d[14];

            sensor_msgs::msg::Imu imu{};
            imu.header.stamp = now();
            imu.header.frame_id = "imu_link";

            sensor_msgs::msg::MagneticField mag{};
            mag.header.stamp = now();
            mag.header.frame_id = "mag_link";

            mag.magnetic_field.x = mx;
            mag.magnetic_field.y = my;
            mag.magnetic_field.z = mz;
            mag.magnetic_field_covariance[0] = -1.0;

            imu.orientation_covariance[0] = -1;
            imu.angular_velocity_covariance[0] = -1;
            imu.linear_acceleration_covariance[0] = -1;

            tf2::Quaternion q;
            q.setRPY(roll, pitch, yaw);

            imu.orientation.x = q.x();
            imu.orientation.y = q.y();
            imu.orientation.z = q.z();
            imu.orientation.w = q.w();

            imu.angular_velocity.x = gx;
            imu.angular_velocity.y = gy;
            imu.angular_velocity.z = gz;

            imu.linear_acceleration.x = ax;
            imu.linear_acceleration.y = ay;
            imu.linear_acceleration.z = az;

            imu_pub_->publish(imu);
            mag_pub_->publish(mag);
            break;
        }

        case ENC_ID :
        {
            float d[4];
            std::memcpy(d, payload_.data() + 4, 4 * sizeof(float));

            float vx = d[0], vy = 0.0f, vz = 0.0f;
            float wx = 0.0f, wy = 0.0f, wz = d[1]; 

            float rpm_vl = d[2], rpm_vr = d[3];
            geometry_msgs::msg::Twist enc_msg{};

            enc_msg.linear.x = vx;
            enc_msg.linear.y = vy;
            enc_msg.linear.z = vz;

            enc_msg.angular.x = wx;
            enc_msg.angular.y = wy;
            enc_msg.angular.z = wz;
            
            enc_pub_->publish(enc_msg);
            break;
        }

        default:
            break;
        }
    }
};

// =====================
// MAIN
// =====================
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StmDriver>());
    rclcpp::shutdown();
    return 0;
}
