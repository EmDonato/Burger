#ifndef SCAN_DATA_H
#define SCAN_DATA_H

#include <stdint.h>
#include <vector>
#include <string>

struct LaserScan {
    double angle_min;
    double angle_max;
    double angle_increment;

    double range_min;
    double range_max;

    double time_increment;
    double scan_time;

    std::vector<float> ranges;
    std::vector<uint16_t> intensities;

    // opzionale: timestamp simulato
    uint64_t stamp;
    std::string frame_id;
};

#endif
