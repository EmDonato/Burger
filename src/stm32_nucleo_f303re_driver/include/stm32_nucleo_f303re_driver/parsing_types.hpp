#pragma once
#include <cstdint>

enum class msg_ID : uint8_t
{
    IMU_ID     = 0x01,
    ENC_ID     = 0x02,
    CMD_VEL_ID = 0x03,
    ARM_ID     = 0x04,
    HB_ID      = 0x05,
    STRING_ID  = 0x06
};

enum class parse_state : uint8_t
{
    WAIT_HEADER1,
    WAIT_HEADER2,
    WAIT_TYPE,
    WAIT_LEN,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM
};

struct msg_info
{
    // FSM
    parse_state state = parse_state::WAIT_HEADER1;
    msg_ID type{};

    // protocol
    uint8_t len = 0;                 
    uint32_t tmp_numb = 0;            

    // counters 
    uint32_t numb_IMU = 0;
    uint32_t numb_ENC = 0;
    uint32_t numb_HB  = 0;

    // payload
    uint8_t payload[128]{};
    uint8_t payload_idx = 0;

    // checksum
    uint8_t checksum = 0;
    bool valid = false;
};
