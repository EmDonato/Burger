#pragma once
#include <Arduino.h>
#include <string.h>


// =============================================================
// === PROTOCOL CONSTANTS ===
// =============================================================
static constexpr uint8_t HDR1 = 0xAA;
static constexpr uint8_t HDR2 = 0x55;
static constexpr uint8_t ODOM_ID = 0x02;

static constexpr uint8_t IMU_ID = 0x01;


inline void sendStringPacket(uint8_t type, const char* str) {

    const uint8_t px1 = 0xAA;
    const uint8_t px2 = 0x55;

    uint8_t len = strlen(str);
    uint8_t checksum = 0;

    Serial.write(px1);  checksum ^= px1;
    Serial.write(px2);  checksum ^= px2;
    Serial.write(type); checksum ^= type;
    Serial.write(len);  checksum ^= len;

    for (uint8_t i = 0; i < len; i++) {
        Serial.write(str[i]);
        checksum ^= str[i];
    }

    Serial.write(checksum);
}


typedef struct{
  uint32_t pwm_L = 0;
  uint32_t pwm_R = 0;
}actualPWM;

// =============================================================
// === SERIAL HELPERS ===
// =============================================================
inline void send_u8(uint8_t v) {
    Serial.write(&v, 1);
}

inline void send_u32(uint32_t v) {
    Serial.write(reinterpret_cast<uint8_t*>(&v), 4);
}

inline void send_f32(float v) {
    Serial.write(reinterpret_cast<uint8_t*>(&v), 4);
}
