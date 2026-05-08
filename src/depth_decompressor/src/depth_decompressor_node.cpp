#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <queue>
#include <thread>

using CompressedImage = sensor_msgs::msg::CompressedImage;
using Image = sensor_msgs::msg::Image;

class DepthDecompressor : public rclcpp::Node
{
public:
  DepthDecompressor()
  : Node("depth_decompressor"),
    running_(true),
    recv_count_(0),
    pub_count_(0),
    drop_count_(0)
  {
    rclcpp::QoS sub_qos(rclcpp::KeepLast(1));
    sub_qos.best_effort();
    sub_qos.durability_volatile();

    rclcpp::QoS pub_qos(rclcpp::KeepLast(1));
    pub_qos.best_effort();
    pub_qos.durability_volatile();

    sub_ = create_subscription<CompressedImage>(
      "/burger/realsense/aligned_depth_to_color/image_raw/compressedDepth",
      sub_qos,
      std::bind(&DepthDecompressor::on_compressed, this, std::placeholders::_1));

    pub_ = create_publisher<Image>(
      "/burger/realsense/aligned_depth_to_color/image_raw/decompressed",
      pub_qos);

    worker_ = std::thread(&DepthDecompressor::decompress_loop, this);

    RCLCPP_INFO(get_logger(), "DepthDecompressor started");
  }

  ~DepthDecompressor()
  {
    {
      std::lock_guard<std::mutex> lk(in_mutex_);
      running_ = false;
    }

    in_cv_.notify_all();

    if (worker_.joinable()) {
      worker_.join();
    }
  }

private:
  static constexpr size_t MAX_IN_QUEUE = 2;

  struct CompressedDepthHeader
  {
    int32_t compression_format;
    float depth_param[2];
  };

  void on_compressed(const CompressedImage::SharedPtr msg)
  {
    ++recv_count_;

    {
      std::lock_guard<std::mutex> lk(in_mutex_);

      while (in_queue_.size() >= MAX_IN_QUEUE) {
        in_queue_.pop();
        ++drop_count_;
      }

      in_queue_.push(msg);
    }

    in_cv_.notify_one();
  }

  static size_t find_png_offset(const std::vector<uint8_t>& data)
  {
    static constexpr uint8_t png_signature[8] =
      {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    auto it = std::search(
      data.begin(),
      data.end(),
      std::begin(png_signature),
      std::end(png_signature));

    if (it == data.end()) {
      return std::string::npos;
    }

    return static_cast<size_t>(std::distance(data.begin(), it));
  }

  static std::string original_encoding_from_format(const std::string& format)
  {
    const auto pos = format.find(';');

    if (pos == std::string::npos) {
      return format;
    }

    return format.substr(0, pos);
  }

  bool parse_compressed_depth_header(
    const CompressedImage& msg,
    CompressedDepthHeader& header) const
  {
    if (msg.data.size() < sizeof(CompressedDepthHeader)) {
      return false;
    }

    std::memcpy(&header, msg.data.data(), sizeof(CompressedDepthHeader));
    return true;
  }

  void decompress_loop()
  {
    while (rclcpp::ok()) {
      CompressedImage::SharedPtr msg;

      {
        std::unique_lock<std::mutex> lk(in_mutex_);
        in_cv_.wait(lk, [this]() {
          return !in_queue_.empty() || !running_;
        });

        if (!running_) {
          break;
        }

        msg = in_queue_.front();
        in_queue_.pop();
      }

      if (!msg) {
        continue;
      }

      RCLCPP_INFO_ONCE(
        get_logger(),
        "Input compressedDepth format: %s",
        msg->format.c_str());

      const size_t png_offset = find_png_offset(msg->data);

      if (png_offset == std::string::npos) {
        ++drop_count_;
        RCLCPP_ERROR(get_logger(), "PNG signature not found in compressedDepth message");
        continue;
      }

      CompressedDepthHeader cd_header{};
      const bool has_cd_header = parse_compressed_depth_header(*msg, cd_header);

      if (has_cd_header) {
        RCLCPP_INFO_ONCE(
          get_logger(),
          "CompressedDepth header: format=%d, param0=%f, param1=%f, png_offset=%zu",
          cd_header.compression_format,
          cd_header.depth_param[0],
          cd_header.depth_param[1],
          png_offset);
      }

      const cv::Mat png_buffer(
        1,
        static_cast<int>(msg->data.size() - png_offset),
        CV_8UC1,
        const_cast<uint8_t*>(msg->data.data() + png_offset));

      cv::Mat decoded = cv::imdecode(png_buffer, cv::IMREAD_UNCHANGED);

      if (decoded.empty()) {
        ++drop_count_;
        RCLCPP_ERROR(get_logger(), "cv::imdecode failed");
        continue;
      }

      const std::string original_encoding =
        original_encoding_from_format(msg->format);

      RCLCPP_INFO_ONCE(
        get_logger(),
        "Decoded PNG: type=%d, size=%dx%d, original_encoding=%s",
        decoded.type(),
        decoded.cols,
        decoded.rows,
        original_encoding.c_str());

      auto out = std::make_unique<Image>();
      out->header = msg->header;
      out->header.frame_id = "camera_color_optical_frame";
      out->height = static_cast<uint32_t>(decoded.rows);
      out->width = static_cast<uint32_t>(decoded.cols);
      out->is_bigendian = false;

      if (original_encoding == "16UC1") {
        if (decoded.type() != CV_16UC1) {
          ++drop_count_;
          RCLCPP_ERROR(
            get_logger(),
            "Expected decoded CV_16UC1 for 16UC1 compressedDepth, got type=%d",
            decoded.type());
          continue;
        }

        out->encoding = sensor_msgs::image_encodings::TYPE_16UC1;
        out->step = static_cast<uint32_t>(decoded.cols * sizeof(uint16_t));
        out->data.resize(static_cast<size_t>(out->step) * out->height);

        if (decoded.isContinuous() && decoded.step == out->step) {
          std::memcpy(out->data.data(), decoded.data, out->data.size());
        } else {
          for (int r = 0; r < decoded.rows; ++r) {
            std::memcpy(
              out->data.data() + static_cast<size_t>(r) * out->step,
              decoded.ptr<uint8_t>(r),
              out->step);
          }
        }
      }
      else if (original_encoding == "32FC1") {
        if (!has_cd_header) {
          ++drop_count_;
          RCLCPP_ERROR(get_logger(), "Missing compressedDepth header for 32FC1 reconstruction");
          continue;
        }

        if (decoded.type() != CV_16UC1) {
          ++drop_count_;
          RCLCPP_ERROR(
            get_logger(),
            "Expected decoded CV_16UC1 inverse-depth image for 32FC1, got type=%d",
            decoded.type());
          continue;
        }

        const float depth_quant_a = cd_header.depth_param[0];
        const float depth_quant_b = cd_header.depth_param[1];

        if (!std::isfinite(depth_quant_a) || !std::isfinite(depth_quant_b) ||
            depth_quant_a == 0.0f) {
          ++drop_count_;
          RCLCPP_ERROR(
            get_logger(),
            "Invalid compressedDepth params: A=%f B=%f",
            depth_quant_a,
            depth_quant_b);
          continue;
        }

        cv::Mat depth_float(decoded.rows, decoded.cols, CV_32FC1);

        for (int r = 0; r < decoded.rows; ++r) {
          const uint16_t* src = decoded.ptr<uint16_t>(r);
          float* dst = depth_float.ptr<float>(r);

          for (int c = 0; c < decoded.cols; ++c) {
            if (src[c] == 0) {
              dst[c] = 0.0f;
            } else {
              dst[c] = depth_quant_a / (static_cast<float>(src[c]) - depth_quant_b);
            }
          }
        }

        out->encoding = sensor_msgs::image_encodings::TYPE_32FC1;
        out->step = static_cast<uint32_t>(depth_float.cols * sizeof(float));
        out->data.resize(static_cast<size_t>(out->step) * out->height);

        std::memcpy(out->data.data(), depth_float.data, out->data.size());
      }
      else {
        ++drop_count_;
        RCLCPP_ERROR(
          get_logger(),
          "Unsupported compressedDepth original encoding: %s",
          original_encoding.c_str());
        continue;
      }

      pub_->publish(std::move(out));
      ++pub_count_;

      RCLCPP_INFO_THROTTLE(
        get_logger(),
        *get_clock(),
        3000,
        "DepthDecompressor stats — recv=%lu pub=%lu drop=%lu",
        recv_count_.load(),
        pub_count_.load(),
        drop_count_.load());
    }
  }

  std::atomic<bool> running_;
  std::atomic<uint64_t> recv_count_;
  std::atomic<uint64_t> pub_count_;
  std::atomic<uint64_t> drop_count_;

  rclcpp::Subscription<CompressedImage>::SharedPtr sub_;
  rclcpp::Publisher<Image>::SharedPtr pub_;

  std::thread worker_;

  std::mutex in_mutex_;
  std::condition_variable in_cv_;
  std::queue<CompressedImage::SharedPtr> in_queue_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DepthDecompressor>());
  rclcpp::shutdown();
  return 0;
}