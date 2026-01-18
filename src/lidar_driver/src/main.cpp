#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

#include <string>
#include <chrono>
#include <mutex>

#include "cmd_interface_linux.h"
#include "lipkg.h"

class LidarDriver : public rclcpp::Node
{
public:
    LidarDriver()
    : Node("ld06_driver"),
      lidar_("ld06_frame")
    {
        // ======================
        // Parameters
        // ======================
        this->declare_parameter<std::string>("port", "/dev/ttyAMA0");
        this->get_parameter("port", port_name_);

        RCLCPP_INFO(
            this->get_logger(),
            "LD06 node starting on port: %s",
            port_name_.c_str()
        );

        // ======================
        // Serial interface
        // ======================
        cmd_port_.SetReadCallback(
            [this](const char *data, size_t len)
            {
                std::lock_guard<std::mutex> lock(lidar_mutex_);
                if (lidar_.Parse(reinterpret_cast<const uint8_t*>(data), len)) {
                    lidar_.AssemblePacket();
                }
            }
        );

        if (!cmd_port_.Open(port_name_)) {
            RCLCPP_FATAL(this->get_logger(), "Unable to open serial port");
            rclcpp::shutdown();
            return;
        }

        RCLCPP_INFO(this->get_logger(), "LD06 serial port opened");

        // ======================
        // Publisher
        // ======================
        scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>(
            "scan",
            rclcpp::SensorDataQoS()
        );

        // ======================
        // Timer (10 Hz)
        // ======================
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&LidarDriver::publish_scan, this)
        );

        RCLCPP_INFO(this->get_logger(), "LD06 driver started");
    }

    ~LidarDriver() override
    {
        cmd_port_.Close();
        RCLCPP_INFO(this->get_logger(), "LD06 serial port closed");
    }

private:
    // ======================
    // Members
    // ======================
    std::string port_name_;

    CmdInterfaceLinux cmd_port_;
    LiPkg lidar_;

    std::mutex lidar_mutex_;

    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    // ======================
    // Publish LaserScan
    // ======================
    void publish_scan()
    {
        std::lock_guard<std::mutex> lock(lidar_mutex_);

        if (!lidar_.IsFrameReady()) {
            return;
        }

        auto scan = lidar_.GetLaserScan();
        lidar_.ResetFrameReady();

        sensor_msgs::msg::LaserScan msg;

        msg.header.stamp = this->now();
        msg.header.frame_id = "laser_link";

        msg.angle_min = scan.angle_min;
        msg.angle_max = scan.angle_max;
        msg.angle_increment = scan.angle_increment;

        msg.range_min = scan.range_min;
        msg.range_max = scan.range_max;

        const size_t count = scan.ranges.size();

        msg.ranges.resize(count);
        msg.intensities.resize(count);

        for (size_t i = 0; i < count; ++i) {
            msg.ranges[i] = scan.ranges[i];
            msg.intensities[i] = scan.intensities[i];
        }

        msg.scan_time = scan.scan_time;                 
        msg.time_increment = msg.scan_time / count;     

        scan_pub_->publish(msg);
    }
};

// ======================
// main
// ======================
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LidarDriver>());
    rclcpp::shutdown();
    return 0;
}


