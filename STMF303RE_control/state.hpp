#pragma once

#include "Task.hpp"
#include <Arduino.h>
#include "freertos_config_tasks.h"
#include "control_config.h"

// =============================================================
// === CONFIGURATION MACROS ===
// =============================================================
#define SEND_HB 1

// =============================================================
// === SHARED STATE ===
// =============================================================
extern uint32_t state;

// =============================================================
// === HB TASK ===
// =============================================================
class HBTask : public TaskBase
{
private:
    uint32_t code_  = 0x00;  // status / error code
    uint32_t state_ = 0x00;
    uint32_t ID_    = 0;

public:
    HBTask(const TaskSpec& s) : TaskBase(s) {}

protected:
    void setup() override
    {
        sendStringPacket(STRING_ID, "[STATE] HeartBeat task started");
    }

    bool wait() override
    {
        return true;
    }

    void loop() override
    {
        state_ = state;

        // =========================
        // BUILD FRAME
        // =========================
        uint8_t checksum = 0;

        auto chk_u8 = [&](uint8_t v) {
            checksum ^= v;
        };
        auto chk_u32 = [&](uint32_t v) {
            uint8_t* p = reinterpret_cast<uint8_t*>(&v);
            for (int i = 0; i < 4; i++) checksum ^= p[i];
        };

#if SEND_HB
        // Header
        chk_u8(HDR1); send_u8(HDR1);
        chk_u8(HDR2); send_u8(HDR2);

        // Type + LEN
        chk_u8(HB_ID);                     send_u8(HB_ID);
        chk_u8(sizeof(uint32_t) * 2);      send_u8(sizeof(uint32_t) * 2);

        // Payload
        chk_u32(code_);  send_u32(code_);
        chk_u32(state_); send_u32(state_);

        // Checksum finale
        send_u8(checksum);
#endif

        ID_++;
    }
};
