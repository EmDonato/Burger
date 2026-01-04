#pragma once
#include <cstdint>

enum class msg_ID
{
    IMU_ID     = 0x01,
    ODOM_ID    = 0x02,
    CMD_VEL_ID = 0x03,
    ARM_ID     = 0x04,
    HB_ID      = 0x05,
    STRING_ID  = 0x06

};

enum class parse_state
{

    WAIT_HEADER1  = 0x01,
    WAIT_HEADER2  = 0x02,
    WAIT_TYPE     = 0x03,
    WAIT_LEN      = 0x04,
    WAIT_PAYLOAD  = 0x05,
    WAIT_CHECKSUM = 0x06

};

typedef struct{

    msg_ID type;
    parse_state state = parse_state::WAIT_HEADER1;
    uint8_t len;
    uint32_t numbMSG = 0;
    uint32_t payload[15];
    bool checksumValid;

}msg_info;


