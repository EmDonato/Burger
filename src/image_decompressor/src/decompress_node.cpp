#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <opencv2/core.hpp>
#include <turbojpeg.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>

using CompressedImage = sensor_msgs::msg::CompressedImage;
using Image           = sensor_msgs::msg::Image;

class DecompressNode : public rclcpp::Node
{
public:
  DecompressNode() : Node("decompress_node"), running_(true)
  {
    tj_ = tjInitDecompress();
    if (!tj_) {
      RCLCPP_FATAL(get_logger(), "tjInitDecompress fallito");
      throw std::runtime_error("turbojpeg init failed");
    }

    rclcpp::QoS sub_qos(1);
    sub_qos.best_effort().durability_volatile();

    rclcpp::QoS pub_qos(1);
    pub_qos.best_effort().durability_volatile();

    sub_ = this->create_subscription<CompressedImage>(
      "/burger/realsense/color/image_raw/compressed", sub_qos,
      [this](CompressedImage::SharedPtr msg) { enqueue(std::move(msg)); });

    pub_ = this->create_publisher<Image>(
      "/burger/realsense/color/image_raw/decompressed", pub_qos);

    worker_ = std::thread(&DecompressNode::workerLoop, this);
    RCLCPP_INFO(get_logger(), "Decompress node avviato (turbojpeg).");
  }

  ~DecompressNode()
  {
    running_ = false;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
    if (tj_) tjDestroy(tj_);
  }

private:
  void enqueue(CompressedImage::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    if (queue_.size() >= MAX_QUEUE) {
      queue_.pop_front();
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "Drop frame");
    }
    queue_.push_back(std::move(msg));
    cv_.notify_one();
  }

  void workerLoop()
  {
    cv::Mat frame;

    while (running_) {
      CompressedImage::SharedPtr msg;
      {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return !queue_.empty() || !running_; });
        if (!running_ && queue_.empty()) break;
        msg = std::move(queue_.front());
        queue_.pop_front();
      }

      if (!msg || msg->data.empty()) continue;

      int width, height, subsamp, colorspace;
      if (tjDecompressHeader3(tj_,
            msg->data.data(), msg->data.size(),
            &width, &height, &subsamp, &colorspace) < 0) {
        RCLCPP_WARN(get_logger(), "Header JPEG not valid: %s", tjGetErrorStr2(tj_));
        continue;
      }

      if (frame.rows != height || frame.cols != width) {
        frame.create(height, width, CV_8UC3);
       
      }

      auto t0 = std::chrono::steady_clock::now();

      if (tjDecompress2(tj_,
            msg->data.data(), msg->data.size(),
            frame.data,
            width, frame.step, height,
            TJPF_BGR,
            TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) < 0) {
        RCLCPP_WARN(get_logger(), "Decode faild: %s", tjGetErrorStr2(tj_));
        continue;
      }

      auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - t0).count();


      auto out = std::make_unique<Image>();
      out->header       = std::move(msg->header);
      out->encoding     = sensor_msgs::image_encodings::BGR8;
      out->is_bigendian = 0;
      out->height       = static_cast<uint32_t>(height);
      out->width        = static_cast<uint32_t>(width);
      out->step         = static_cast<uint32_t>(frame.step);
      auto ta = std::chrono::steady_clock::now();
      out->data.assign(frame.data, frame.data + frame.step * height);
      auto tb = std::chrono::steady_clock::now();
      pub_->publish(std::move(*out));
      auto tc = std::chrono::steady_clock::now();
    }
  }

  static constexpr size_t MAX_QUEUE = 2;

  tjhandle tj_{nullptr};
  rclcpp::Subscription<CompressedImage>::SharedPtr sub_;
  rclcpp::Publisher<Image>::SharedPtr              pub_;
  std::thread              worker_;
  std::mutex               mtx_;
  std::condition_variable  cv_;
  std::deque<CompressedImage::SharedPtr> queue_;
  std::atomic<bool>        running_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DecompressNode>());
  rclcpp::shutdown();
  return 0;
}