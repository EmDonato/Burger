#ifndef __LIPKG_H
#define __LIPKG_H

#include <stdint.h>
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "pointdata.h"
#include "scan_data.h"

#define ANGLE_TO_RADIAN(angle) ((angle)*3.14159 / 180.0)

enum {
  PKG_HEADER = 0x54,
  PKG_VER_LEN = 0x2C,
  POINT_PER_PACK = 12,
};

typedef struct __attribute__((packed)) {
  uint16_t distance;
  uint8_t confidence;
} LidarPointStructDef;

typedef struct __attribute__((packed)) {
  uint8_t header;
  uint8_t ver_len;
  uint16_t speed;
  uint16_t start_angle;
  LidarPointStructDef point[POINT_PER_PACK];
  uint16_t end_angle;
  uint16_t timestamp;
  uint8_t crc8;
} LiDARFrameTypeDef;

class LiPkg {
 public:
  LiPkg(std::string frame_id);
  
  double GetSpeed(void);
  uint16_t GetTimestamp(void) { return timestamp_; }
  
  bool IsPkgReady(void) { return is_pkg_ready_; }
  bool IsFrameReady(void) { return is_frame_ready_; }
  void ResetFrameReady(void) { is_frame_ready_ = false; }
  
  long GetErrorTimes(void) { return error_times_; }
  
  bool AnalysisOne(uint8_t byte);
  bool Parse(const uint8_t* data, long len);
  bool AssemblePacket();

  // 🔥 NOSTRO LaserScan custom
  LaserScan GetLaserScan() { return output; }

 private:
  const int kPointFrequence = 4500;
  const std::array<PointData, POINT_PER_PACK>& GetPkgData(void);

  std::string frame_id_;
  uint16_t timestamp_;
  double speed_;
  long error_times_;

  LiDARFrameTypeDef pkg;
  std::array<PointData, POINT_PER_PACK> one_pkg_;
  std::vector<PointData> frame_tmp_;

  bool is_pkg_ready_;
  bool is_frame_ready_;

  LaserScan output;

  void ToLaserscan(std::vector<PointData> src);
};

#endif
