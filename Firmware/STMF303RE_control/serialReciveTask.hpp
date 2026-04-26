#pragma once

#include "Task.hpp"
#include <Arduino.h>
#include <string.h>
#include "math_function.h"

extern uint32_t state;

class SerialReceiveTask : public TaskBase {

private:
    enum ParseState {
        WAIT_HEADER1,
        WAIT_HEADER2,
        WAIT_LEN,
        WAIT_TYPE,
        WAIT_PAYLOAD,
        WAIT_CHECKSUM
    };

    static constexpr uint8_t MAX_PAYLOAD_SIZE = 16;

    ParseState state_ = WAIT_HEADER1;

    bool package_read_ = false;

    uint8_t len_ = 0;
    uint8_t type_ = 0;

    uint8_t payload_[MAX_PAYLOAD_SIZE];
    uint8_t payload_index_ = 0;

    uint8_t checksum_calc_ = 0;
    QueueHandle_t queues_;
public:
    SerialReceiveTask(const TaskSpec& s, QueueHandle_t ref_q)
        : TaskBase(s),
        queues_(ref_q) {}

protected:
    void setup() override {
          sendStringPacket(STRING_ID,"[SERIAL] Serial task started");
        return;
    }

    bool wait() override {
        return true;
    }

    void loop() override {

        package_read_ = false;

        while (Serial.available() && !package_read_) {
            uint8_t c = Serial.read();
            parseByte(c);
        }
    }

private:
    void parseByte(uint8_t c) {

        switch (state_) {

        case WAIT_HEADER1:
            if (c == 0xAA) {
                checksum_calc_ = c;
                state_ = WAIT_HEADER2;
            }
            break;

        case WAIT_HEADER2:
            if (c == 0x55) {
                checksum_calc_ ^= c;
                state_ = WAIT_LEN;
            } else {
                state_ = WAIT_HEADER1;
            }
            break;

        case WAIT_LEN:
            len_ = c;
            payload_index_ = 0;
            checksum_calc_ ^= c;

            if (len_ > MAX_PAYLOAD_SIZE) {
                state_ = WAIT_HEADER1;   
            } else {
                state_ = WAIT_TYPE;
            }
            break;

        case WAIT_TYPE:
            type_ = c;
            checksum_calc_ ^= c;
            state_ = (len_ == 0) ? WAIT_CHECKSUM : WAIT_PAYLOAD;
            break;

        case WAIT_PAYLOAD:
            payload_[payload_index_++] = c;
            checksum_calc_ ^= c;

            if (payload_index_ >= len_) {
                state_ = WAIT_CHECKSUM;
            }
            break;

        case WAIT_CHECKSUM:
            if (checksum_calc_ == c) {
                handleMessage(type_, payload_, len_);
            }
            state_ = WAIT_HEADER1;
            package_read_ = true;
            break;
        }
    }

    // ===========================
    // GESTIONE DEI MESSAGGI
    // ===========================
    void handleMessage(uint8_t type, const uint8_t* payload, uint8_t len) {

        switch (type) {

        case ARM_ID:  // EVENTO
        {
            state ++;
            state = state % 2;
          if (state == 1) {
              digitalWrite(LED_GREEN, HIGH);
          } else {
              digitalWrite(LED_GREEN, LOW);
          }
            break;
        }

        case CMD_VEL_ID:  // REFERENCE (2 float)
        {
            if (len != sizeof(RefMsg)) {
                return;
            }

            RefMsg ref;
            memcpy(&ref, payload, sizeof(RefMsg));

            xQueueOverwrite(queues_, &ref);
            break;
        }

        default:
            // tipo sconosciuto
            break;
        }
    }
};

