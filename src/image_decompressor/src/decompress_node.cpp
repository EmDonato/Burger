#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <turbojpeg.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

using CompressedImage = sensor_msgs::msg::CompressedImage;
using Image = sensor_msgs::msg::Image;

class DecompressNode : public rclcpp::Node
{
public:
  explicit DecompressNode(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("decompress_node", options)
  {
    tj_handle_ = tjInitDecompress();

    if (tj_handle_ == nullptr) {
      RCLCPP_FATAL(get_logger(), "tjInitDecompress fallito");
      throw std::runtime_error("Impossibile inizializzare TurboJPEG");
    }

    rclcpp::QoS sub_qos(rclcpp::KeepLast(1));
    sub_qos.best_effort().durability_volatile();

    rclcpp::QoS pub_qos(rclcpp::KeepLast(1));
    pub_qos.best_effort().durability_volatile();

    publisher_ = create_publisher<Image>(
      "/burger/realsense/color/image_raw/decompressed",
      pub_qos);

    subscription_ = create_subscription<CompressedImage>(
      "/burger/realsense/color/image_raw/compressed",
      sub_qos,
      [this](CompressedImage::SharedPtr message) {
        receiveFrame(std::move(message));
      });

    worker_thread_ = std::thread(
      &DecompressNode::workerLoop,
      this);

    RCLCPP_INFO(get_logger(), "Decompressore TurboJPEG avviato");
  }

  ~DecompressNode() override
  {
    running_.store(false, std::memory_order_release);
    condition_variable_.notify_all();

    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }

    if (tj_handle_ != nullptr) {
      tjDestroy(tj_handle_);
      tj_handle_ = nullptr;
    }
  }

private:
  void receiveFrame(CompressedImage::SharedPtr message)
  {
    if (!message || message->data.empty()) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);

      if (pending_message_ != nullptr) {
        dropped_frames_.fetch_add(1, std::memory_order_relaxed);
      }

      pending_message_ = std::move(message);
    }

    condition_variable_.notify_one();
  }

  void workerLoop()
  {
    while (true) {
      CompressedImage::SharedPtr compressed_message;

      {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_variable_.wait(
          lock,
          [this]() {
            return pending_message_ != nullptr ||
                   !running_.load(std::memory_order_acquire);
          });

        if (!running_.load(std::memory_order_acquire) &&
            pending_message_ == nullptr)
        {
          break;
        }

        compressed_message = std::move(pending_message_);
      }

      if (!compressed_message || compressed_message->data.empty()) {
        continue;
      }

      decodeAndPublish(compressed_message);
    }
  }

  void decodeAndPublish(
    const CompressedImage::SharedPtr & compressed_message)
  {
    int width = 0;
    int height = 0;
    int subsampling = 0;
    int colorspace = 0;

    const auto * jpeg_data = compressed_message->data.data();

    const auto jpeg_size = static_cast<unsigned long>(
      compressed_message->data.size());

    if (tjDecompressHeader3(
          tj_handle_,
          jpeg_data,
          jpeg_size,
          &width,
          &height,
          &subsampling,
          &colorspace) < 0)
    {
      RCLCPP_WARN_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Header JPEG non valido: %s",
        tjGetErrorStr2(tj_handle_));

      return;
    }

    if (width <= 0 || height <= 0) {
      RCLCPP_WARN_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Dimensioni JPEG non valide: %dx%d",
        width,
        height);

      return;
    }

    auto output_message = std::make_unique<Image>();

    output_message->header = compressed_message->header;
    output_message->height = static_cast<uint32_t>(height);
    output_message->width = static_cast<uint32_t>(width);
    output_message->encoding = sensor_msgs::image_encodings::BGR8;
    output_message->is_bigendian = false;
    output_message->step = static_cast<uint32_t>(width * 3);

    const std::size_t output_size =
      static_cast<std::size_t>(output_message->step) *
      static_cast<std::size_t>(output_message->height);

    output_message->data.resize(output_size);

    const auto decode_start = std::chrono::steady_clock::now();

    if (tjDecompress2(
          tj_handle_,
          jpeg_data,
          jpeg_size,
          output_message->data.data(),
          width,
          static_cast<int>(output_message->step),
          height,
          TJPF_BGR,
          TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) < 0)
    {
      RCLCPP_WARN_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Decompressione JPEG fallita: %s",
        tjGetErrorStr2(tj_handle_));

      return;
    }

    const auto decode_end = std::chrono::steady_clock::now();

    const auto decode_microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(
        decode_end - decode_start).count();

    const uint64_t decoded_frames =
      decoded_frames_.fetch_add(1, std::memory_order_relaxed) + 1;

    publisher_->publish(std::move(output_message));

    if (decoded_frames % 100 == 0) {
      const uint64_t dropped_frames =
        dropped_frames_.load(std::memory_order_relaxed);

      RCLCPP_INFO(
        get_logger(),
        "Decodificati: %lu | Sostituiti: %lu | Decode: %.3f ms",
        static_cast<unsigned long>(decoded_frames),
        static_cast<unsigned long>(dropped_frames),
        static_cast<double>(decode_microseconds) / 1000.0);
    }
  }

  tjhandle tj_handle_{nullptr};

  rclcpp::Subscription<CompressedImage>::SharedPtr subscription_;
  rclcpp::Publisher<Image>::SharedPtr publisher_;

  std::thread worker_thread_;
  std::mutex mutex_;
  std::condition_variable condition_variable_;

  CompressedImage::SharedPtr pending_message_;

  std::atomic<bool> running_{true};
  std::atomic<uint64_t> decoded_frames_{0};
  std::atomic<uint64_t> dropped_frames_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.use_intra_process_comms(true);

  auto node = std::make_shared<DecompressNode>(options);

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
