#include <opencv2/opencv.hpp>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>

#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <cstring>
//THAT'S SHIT
using namespace std::chrono_literals;

class Module3 : public rclcpp::Node {
public:
    Module3() : Node("module3") {

        width_ = 1280;
        height_ = 720;
        fps_ = 30;
        jpeg_quality_ = 70;
        lb_size_ = 640;

        std::string pipeline =
            "libcamerasrc ! "
            "video/x-raw,width=1280,height=720,framerate=30/1,format=NV12 ! "
            "videoconvert ! video/x-raw,format=BGR ! "
            "appsink drop=true max-buffers=1 sync=false";

        cap_.open(pipeline, cv::CAP_GSTREAMER);

        if (!cap_.isOpened()) {
            throw std::runtime_error("Camera open failed");
        }

        frame_a_.create(height_, width_, CV_8UC3);
        frame_b_.create(height_, width_, CV_8UC3);

        lb_frame_.create(lb_size_, lb_size_, CV_8UC3);

        auto qos = rclcpp::SensorDataQoS();

        pub_compressed_ = create_publisher<sensor_msgs::msg::CompressedImage>(
            "burger/module_3/color/image_raw/compressed", qos);

        pub_letterbox_ = create_publisher<sensor_msgs::msg::Image>(
            "burger/module_3/color/image_letterbox", qos);

        timer_fast_ = create_wall_timer(
            33ms, std::bind(&Module3::fast_loop, this));

        timer_slow_ = create_wall_timer(
            33ms, std::bind(&Module3::slow_loop, this));

        jpeg_buffer_.reserve(width_ * height_ / 4);

        RCLCPP_INFO(get_logger(), "Module3 FINAL (single cam) started");
    }

private:

    void fast_loop() {

        cv::Mat frame;

        if (!cap_.read(frame) || frame.empty()) return;

        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (use_a_) {
                frame.copyTo(frame_a_);
                latest_ = &frame_a_;
            } else {
                frame.copyTo(frame_b_);
                latest_ = &frame_b_;
            }
            use_a_ = !use_a_;
        }

        if (pub_compressed_->get_subscription_count() == 0) return;

        jpeg_buffer_.clear();

        cv::imencode(".jpg", frame, jpeg_buffer_,
                     {cv::IMWRITE_JPEG_QUALITY, jpeg_quality_});

        sensor_msgs::msg::CompressedImage msg;
        msg.header.stamp = now();
        msg.header.frame_id = "camera_rgb_link";
        msg.format = "jpeg";
        msg.data = std::move(jpeg_buffer_);

        pub_compressed_->publish(std::move(msg));

        jpeg_buffer_.clear();
    }

    void slow_loop() {

        if (pub_letterbox_->get_subscription_count() == 0) return;

        cv::Mat frame;

        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (!latest_) return;
            frame = *latest_;
        }

        float scale = (float)lb_size_ /
                      (float)std::max(frame.rows, frame.cols);

        int new_w = (int)(frame.cols * scale);
        int new_h = (int)(frame.rows * scale);

        int pad_top  = (lb_size_ - new_h) / 2;
        int pad_left = (lb_size_ - new_w) / 2;

        lb_frame_.setTo(cv::Scalar(114,114,114));

        cv::Mat roi = lb_frame_(cv::Rect(pad_left, pad_top, new_w, new_h));

        cv::resize(frame, roi, cv::Size(new_w, new_h), 0, 0, cv::INTER_LINEAR);

        sensor_msgs::msg::Image msg;

        msg.header.stamp = now();
        msg.header.frame_id =
            "letterbox:" +
            std::to_string(scale) + ";" +
            std::to_string(pad_top) + ";" +
            std::to_string(pad_left);

        msg.height = lb_frame_.rows;
        msg.width  = lb_frame_.cols;
        msg.encoding = "bgr8";
        msg.step = lb_frame_.cols * 3;

        size_t size = lb_frame_.total() * 3;
        msg.data.resize(size);
        std::memcpy(msg.data.data(), lb_frame_.data, size);

        pub_letterbox_->publish(std::move(msg));
    }

    // ───────── DATA ─────────
    int width_, height_, fps_;
    int jpeg_quality_;
    int lb_size_;

    cv::VideoCapture cap_;

    cv::Mat frame_a_, frame_b_;
    cv::Mat* latest_{nullptr};
    bool use_a_{true};

    cv::Mat lb_frame_;

    std::vector<uchar> jpeg_buffer_;

    std::mutex mutex_;

    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_compressed_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_letterbox_;

    rclcpp::TimerBase::SharedPtr timer_fast_;
    rclcpp::TimerBase::SharedPtr timer_slow_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Module3>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}