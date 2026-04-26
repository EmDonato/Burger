#pragma once

#include <STM32FreeRTOS.h>
#include "Task.hpp"
#include "control_config.h"
#include <cmath>
#include "math_function.h"
#include "MotorDriver.h"
#include "PID.h"
#define VERBOSE_CONTROL 0

// === Oggetti globali definiti nel main ===
extern PID pid_L;
extern PID pid_R;
extern L298N *motor;   
extern  uint32_t state;
extern actualPWM pwm_motors;

class controlTask : public TaskBase {

private:
    QueueHandle_t* queue_;

    RefMsg ref_msg_;
    WheelRpmFeedback fd_msg_;

    float v_ref_ = 0.0f;
    float w_ref_ = 0.0f;

    float v_l_actual_rpm_ = 0.0f;
    float v_r_actual_rpm_ = 0.0f;
    int uL, uR;

public:
    controlTask(const TaskSpec& s, QueueHandle_t* queues)
        : TaskBase(s), queue_(queues) {}

protected:
    void setup() override
    {
      sendStringPacket(STRING_ID,"[CONTROL] control task started");
    }

    void loop() override
    {
        //Serial.println("[CONTROL] in loop");
        // === 1. Ricezione reference ===
        if (xQueueReceive(queue_[0], &ref_msg_, 0) == pdPASS) {
            v_ref_ = ref_msg_.v_ref;
            w_ref_ = ref_msg_.w_ref;
        }

        // === 2. Ricezione feedback encoder ===
        if (xQueueReceive(queue_[1], &fd_msg_, 0) == pdPASS) {
            v_l_actual_rpm_ = fd_msg_.vl_rpm;
            v_r_actual_rpm_ = fd_msg_.vr_rpm;
        }

        // === 3. Cinematica inversa ===
        WheelSpeeds w_ref = compute_wheel_speeds(v_ref_/V_MAX, w_ref_/W_MAX);

        float w_L_ref =  w_ref.left;
        float w_R_ref =  w_ref.right;

        WheelSpeeds rpm_ref = computeRPM(w_L_ref, w_R_ref);

        float rpm_ref_L = rpm_ref.left;
        float rpm_ref_R = rpm_ref.right;

        constexpr float EPS_THRESHOLD = 0.01f;

        // === 4. Ruota sinistra ===
        if (std::abs(w_L_ref) > EPS_THRESHOLD) {

             uL = std::abs(
                pid_L.compute(
                    std::abs(rpm_ref_L),
                    std::abs(v_l_actual_rpm_)
                )
            );

            if (w_L_ref > 0) {
                motor->forward(uL, 0);
            } else {
                motor->backward(uL, 0);
            }

#if VERBOSE_CONTROL
            Serial.print("[L] ref/act/u: ");
            Serial.print(rpm_ref_L);
            Serial.print(" ");
            Serial.print(v_l_actual_rpm_);
            Serial.print(" ");
            Serial.println(uL);
#endif

        } else {
            motor->forward(0, 0);
        }

        // === 5. Ruota destra ===
        if (std::abs(w_R_ref) > EPS_THRESHOLD) {

            uR = std::abs(
                pid_R.compute(
                    std::abs(rpm_ref_R),
                    std::abs(v_r_actual_rpm_)
                )
            );

            if (w_R_ref > 0) {
                motor->forward(uR, 1);
            } else {
                motor->backward(uR, 1);
            }

#if VERBOSE_CONTROL
            Serial.print("[R] ref/act/u: ");
            Serial.print(rpm_ref_R);
            Serial.print(" ");
            Serial.print(v_r_actual_rpm_);
            Serial.print(" ");
            Serial.println(uR);
#endif

        } else {
            motor->forward(0, 1);
        }
    pwm_motors.pwm_L = (uint32_t)uL;
    pwm_motors.pwm_R = (uint32_t)uR;
        }


    bool wait() override
    {
        if (state == 0){
          motor->forward(0, 0);
          return false;
          } 
        else  return true;
    }
};

