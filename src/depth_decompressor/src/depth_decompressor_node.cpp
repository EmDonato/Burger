#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

using CompressedImage = sensor_msgs::msg::CompressedImage;
using Image = sensor_msgs::msg::Image;

class DepthDecompressor : public rclcpp::Node
{
public:
  explicit DepthDecompressor(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("depth_decompressor", options)
  {
    rclcpp::QoS sub_qos(rclcpp::KeepLast(1));
    sub_qos.best_effort().durability_volatile();

    rclcpp::QoS pub_qos(rclcpp::KeepLast(1));
    pub_qos.best_effort().durability_volatile();

    publisher_ = create_publisher<Image>(
      "/burger/realsense/aligned_depth_to_color/image_raw/decompressed",
      pub_qos);

    subscription_ = create_subscription<CompressedImage>(
      "/burger/realsense/aligned_depth_to_color/image_raw/compressedDepth",
      sub_qos,
      [this](CompressedImage::SharedPtr message) {
        receiveFrame(std::move(message));
      });

    worker_thread_ = std::thread(
      &DepthDecompressor::workerLoop,
      this);

    RCLCPP_INFO(
      get_logger(),
      "DepthDecompressor avviato");
  }

  ~DepthDecompressor() override
  {
    running_.store(false, std::memory_order_release);
    condition_variable_.notify_all();

    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }
  }

private:
  enum CompressionFormat : int32_t
  {
    UNDEFINED = -1,
    INV_DEPTH = 0
  };

  struct CompressedDepthHeader
  {
    CompressionFormat format;
    float depth_param[2];
  };

  void receiveFrame(CompressedImage::SharedPtr message)
  {
    if (!message || message->data.empty()) {
      return;
    }

    received_frames_.fetch_add(1, std::memory_order_relaxed);

    {
      std::lock_guard<std::mutex> lock(mutex_);

      if (pending_message_) {
        dropped_frames_.fetch_add(1, std::memory_order_relaxed);
      }

      pending_message_ = std::move(message);
    }

    condition_variable_.notify_one();
  }

  void workerLoop()
  {
    while (true) {
      CompressedImage::SharedPtr message;

      {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_variable_.wait(
          lock,
          [this]() {
            return pending_message_ ||
                   !running_.load(std::memory_order_acquire);
          });

        if (!running_.load(std::memory_order_acquire) &&
            !pending_message_)
        {
          break;
        }

        message = std::move(pending_message_);
      }

      if (!message || message->data.empty()) {
        continue;
      }

      decodeAndPublish(*message);
    }
  }

  void decodeAndPublish(const CompressedImage & message)
  {
    RCLCPP_INFO_ONCE(
      get_logger(),
      "Formato compressedDepth: %s",
      message.format.c_str());

    const std::string encoding =
      originalEncoding(message.format);

    if (encoding != sensor_msgs::image_encodings::TYPE_16UC1 &&
        encoding != sensor_msgs::image_encodings::TYPE_32FC1)
    {
      dropped_frames_.fetch_add(1, std::memory_order_relaxed);

      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Encoding compressedDepth non supportato: %s",
        encoding.c_str());

      return;
    }

    if (message.data.size() <= sizeof(CompressedDepthHeader)) {
      dropped_frames_.fetch_add(1, std::memory_order_relaxed);

      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Messaggio compressedDepth troppo piccolo");

      return;
    }

    CompressedDepthHeader compressed_header{};

    std::memcpy(
      &compressed_header,
      message.data.data(),
      sizeof(compressed_header));

    const auto * compressed_data =
      message.data.data() + sizeof(compressed_header);

    const std::size_t compressed_size =
      message.data.size() - sizeof(compressed_header);

    if (!hasPngSignature(compressed_data, compressed_size)) {
      dropped_frames_.fetch_add(1, std::memory_order_relaxed);

      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Firma PNG non valida dopo il compressedDepth header");

      return;
    }

    const cv::Mat compressed_buffer(
      1,
      static_cast<int>(compressed_size),
      CV_8UC1,
      const_cast<uint8_t *>(compressed_data));

    const auto decode_start =
      std::chrono::steady_clock::now();

    cv::Mat decoded = cv::imdecode(
      compressed_buffer,
      cv::IMREAD_UNCHANGED,
      &decode_buffer_);

    if (decoded.empty()) {
      dropped_frames_.fetch_add(1, std::memory_order_relaxed);

      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "cv::imdecode fallito");

      return;
    }

    if (decoded.type() != CV_16UC1) {
      dropped_frames_.fetch_add(1, std::memory_order_relaxed);

      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Immagine PNG decodificata con tipo errato: %d",
        decoded.type());

      return;
    }

    auto output = std::make_unique<Image>();

    output->header = message.header;
    output->height = static_cast<uint32_t>(decoded.rows);
    output->width = static_cast<uint32_t>(decoded.cols);
    output->is_bigendian = false;

    if (encoding == sensor_msgs::image_encodings::TYPE_16UC1) {
      fill16UC1(decoded, *output);
    } else {
      if (!fill32FC1(
            decoded,
            compressed_header.depth_param[0],
            compressed_header.depth_param[1],
            *output))
      {
        dropped_frames_.fetch_add(1, std::memory_order_relaxed);
        return;
      }
    }

    const auto decode_end =
      std::chrono::steady_clock::now();

    const auto elapsed_us =
      std::chrono::duration_cast<std::chrono::microseconds>(
        decode_end - decode_start).count();

    publisher_->publish(std::move(output));

    const uint64_t published =
      published_frames_.fetch_add(
        1,
        std::memory_order_relaxed) + 1;

    if ((published % 100U) == 0U) {
      RCLCPP_INFO(
        get_logger(),
        "Ricevuti: %llu | Pubblicati: %llu | Scartati: %llu | Decode: %.3f ms",
        static_cast<unsigned long long>(
          received_frames_.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(published),
        static_cast<unsigned long long>(
          dropped_frames_.load(std::memory_order_relaxed)),
        static_cast<double>(elapsed_us) / 1000.0);
    }
  }

  void fill16UC1(
    const cv::Mat & decoded,
    Image & output)
  {
    output.encoding =
      sensor_msgs::image_encodings::TYPE_16UC1;

    output.step =
      static_cast<uint32_t>(
        decoded.cols * sizeof(uint16_t));

    output.data.resize(
      static_cast<std::size_t>(output.step) *
      output.height);

    if (decoded.isContinuous() &&
        decoded.step == output.step)
    {
      std::memcpy(
        output.data.data(),
        decoded.data,
        output.data.size());

      return;
    }

    for (int row = 0; row < decoded.rows; ++row) {
      std::memcpy(
        output.data.data() +
          static_cast<std::size_t>(row) * output.step,
        decoded.ptr(row),
        output.step);
    }
  }

  bool fill32FC1(
    const cv::Mat & decoded,
    float depth_quant_a,
    float depth_quant_b,
    Image & output)
  {
    if (!std::isfinite(depth_quant_a) ||
        !std::isfinite(depth_quant_b) ||
        depth_quant_a == 0.0F)
    {
      RCLCPP_ERROR_THROTTLE(
        get_logger(),
        *get_clock(),
        2000,
        "Parametri compressedDepth non validi: A=%f B=%f",
        depth_quant_a,
        depth_quant_b);

      return false;
    }

    output.encoding =
      sensor_msgs::image_encodings::TYPE_32FC1;

    output.step =
      static_cast<uint32_t>(
        decoded.cols * sizeof(float));

    output.data.resize(
      static_cast<std::size_t>(output.step) *
      output.height);

    const float invalid_depth =
      std::numeric_limits<float>::quiet_NaN();

    for (int row = 0; row < decoded.rows; ++row) {
      const auto * source =
        decoded.ptr<uint16_t>(row);

      auto * destination =
        reinterpret_cast<float *>(
          output.data.data() +
          static_cast<std::size_t>(row) * output.step);

      for (int column = 0; column < decoded.cols; ++column) {
        const uint16_t inverse_depth = source[column];

        if (inverse_depth == 0U) {
          destination[column] = invalid_depth;
        } else {
          destination[column] =
            depth_quant_a /
            (static_cast<float>(inverse_depth) -
             depth_quant_b);
        }
      }
    }

    return true;
  }

  static bool hasPngSignature(
    const uint8_t * data,
    std::size_t size)
  {
    static constexpr uint8_t signature[8] = {
      0x89, 0x50, 0x4E, 0x47,
      0x0D, 0x0A, 0x1A, 0x0A
    };

    return size >= sizeof(signature) &&
           std::memcmp(
             data,
             signature,
             sizeof(signature)) == 0;
  }

  static std::string originalEncoding(
    const std::string & format)
  {
    const std::size_t separator = format.find(';');

    if (separator == std::string::npos) {
      return format;
    }

    return format.substr(0, separator);
  }

  rclcpp::Subscription<CompressedImage>::SharedPtr subscription_;
  rclcpp::Publisher<Image>::SharedPtr publisher_;

  std::thread worker_thread_;
  std::mutex mutex_;
  std::condition_variable condition_variable_;

  CompressedImage::SharedPtr pending_message_;

  cv::Mat decode_buffer_;

  std::atomic<bool> running_{true};
  std::atomic<uint64_t> received_frames_{0};
  std::atomic<uint64_t> published_frames_{0};
  std::atomic<uint64_t> dropped_frames_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.use_intra_process_comms(true);

  auto node =
    std::make_shared<DepthDecompressor>(options);

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
