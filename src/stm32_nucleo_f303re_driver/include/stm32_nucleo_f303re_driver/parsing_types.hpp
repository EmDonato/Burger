#pragma once
#include <cstdint>

// =====================
// PROTOCOL CONSTANTS
// =====================
static constexpr uint8_t HDR1 = 0xAA;
static constexpr uint8_t HDR2 = 0x55;

enum MsgID : uint8_t {
    IMU_ID     = 0x01,
    ENC_ID     = 0x02,
    CMD_VEL_ID = 0x03,
    ARM_ID     = 0x04,
    HB_ID      = 0x05,
    STRING_ID  = 0x06,
    RESET_ID  = 0x07

};

enum class ParseState {
    WAIT_HEADER1,
    WAIT_HEADER2,
    WAIT_TYPE,
    WAIT_LEN,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM
};

