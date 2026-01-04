#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "stm32_nucleo_f303re_driver/msg/wheel_pwm.hpp"

#include <chrono>
#include <boost/circular_buffer.hpp>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <cerrno>


class StmDriver : public rclcpp::Node
{
public:
    StmDriver()
    : Node("stm32_nucleo_f303re_driver"),
      rx_buffer_(256)
    {
        // =====================
        // PARAMETERS
        // =====================
        this->declare_parameter<std::string>("port", "/dev/ttyACM0");
        this->declare_parameter<int>("baudrate", 115200);
        this->declare_parameter<int>("frequency_ms", 5);

        this->get_parameter("port", port_);
        this->get_parameter("baudrate", baudrate_);
        this->get_parameter("frequency_ms", frequency_ms_);

        // =====================
        // OPEN SERIAL
        // =====================
        serial_fd_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (serial_fd_ < 0) {
            perror("open serial");
            throw std::runtime_error("Failed to open serial port");
        }

        // =====================
        // TERMIOS CONFIG
        // =====================
        termios tty{};
        if (tcgetattr(serial_fd_, &tty) != 0) {
            perror("tcgetattr");
            throw std::runtime_error("tcgetattr failed");
        }

        cfsetispeed(&tty, baud_to_flag(baudrate_));
        cfsetospeed(&tty, baud_to_flag(baudrate_));

        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        tty.c_lflag = 0;
        tty.c_iflag = 0;
        tty.c_oflag = 0;

        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 0;

        if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0) {
            perror("tcsetattr");
            throw std::runtime_error("tcsetattr failed");
        }

        // =====================
        // POLL SETUP
        // =====================
        pfd_.fd = serial_fd_;
        pfd_.events = POLLIN;

        // =====================
        // ROS INTERFACES
        // =====================
        imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>(
            "imu", rclcpp::SensorDataQoS());

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(frequency_ms_),
            std::bind(&StmDriver::read_serial, this)
        );
    }

    ~StmDriver()
    {
        if (serial_fd_ >= 0)
            close(serial_fd_);
    }

private:
    // =====================
    // PARAMETERS
    // =====================
    std::string port_;
    int baudrate_;
    int frequency_ms_;

    // =====================
    // SERIAL
    // =====================
    int serial_fd_{-1};
    pollfd pfd_{};

    // =====================
    // BUFFERS
    // =====================
    boost::circular_buffer<uint8_t> rx_buffer_(512);
    uint8_t tmp_buf_[128];

    // =====================
    // ROS
    // =====================
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    // =====================
    // TIMER CALLBACK
    // =====================
    void read_serial()
    {
        // Consuma TUTTO quello che il kernel ha bufferizzato
        while (poll_and_read_()) {
            // intenzionalmente vuoto
            // qui NON parsare: solo I/O
        }

        // Qui dopo andrai con:
        // parse_rx_buffer(rx_buffer_);
    }

    // =====================
    // POLL + READ
    // =====================
    bool poll_and_read_()
    {
        int ret = poll(&pfd_, 1, 0);  // non blocca

        if (ret > 0 && (pfd_.revents & POLLIN)) {

            ssize_t n = read(serial_fd_, tmp_buf_, sizeof(tmp_buf_));

            if (n > 0) {
                for (ssize_t i = 0; i < n; ++i) {
                    rx_buffer_.push_back(tmp_buf_[i]);
                }
                return true;   // c'era dato → riprova
            }
        }

        return false; // niente altro da leggere
    }

    // =====================
    // BAUDRATE HELPER
    // =====================
    speed_t baud_to_flag(int baud)
    {
        switch (baud) {
            case 9600:   return B9600;
            case 57600:  return B57600;
            case 115200: return B115200;
            case 230400: return B230400;
            default:     return B115200;
        }
    }
};
