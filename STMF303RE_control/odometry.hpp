#pragma once

#include "Task.hpp"
#include "encoder.h"
#include "EncoderPoseEstimator.hpp"
#include "odometry_class.hpp"
#include <Arduino.h>

#define VERBOSE_ODOM 0
#define SEND_ODOM    1

// =============================================================
// === EXTERNAL OBJECTS ===
// =============================================================
extern Encoder encoder_L;
extern Encoder encoder_R;
extern UnicycleOdometry odometry;
extern EncoderPoseEstimator Enc_est;
extern actualPWM pwm_motors;



// payload = idMsg + vlong + w + vl_rpm + vr_rpm + pwm_l + pwm_r
static constexpr uint8_t ODOM_PAYLOAD_LEN =
    sizeof(uint32_t) +        // idMsg
    2 * sizeof(float) +       // vlong_est, w_est
    2 * sizeof(float) +       // vl_rpm, vr_rpm
    2 * sizeof(uint32_t);     // pwm_l, pwm_r

// =============================================================
// === ODOMETRY TASK ===
// =============================================================
class OdometryTask : public TaskBase
{
private:
    float vlong_est_, w_est_;
    float vl_rpm_, vr_rpm_;
    uint32_t pwm_l_, pwm_r_;

    const float dt_ = TASK_ODOM_PERIOD_MS / 1000.0f;
    uint32_t ID_ = 0;
    QueueHandle_t feedback_q_;

public:
    OdometryTask(const TaskSpec& s, QueueHandle_t feedback_q)
        : TaskBase(s), feedback_q_(feedback_q) {}

protected:
    void setup() override
    {
        sendStringPacket(STRING_ID, "[ODOMETRY] odometry task started");
    }

    bool wait() override
    {
        return true;
    }

    void loop() override
    {
        // === Read encoders ===
        vl_rpm_ = encoder_L.getOutputRPMandReset(TASK_ODOM_PERIOD_MS);
        vr_rpm_ = encoder_R.getOutputRPMandReset(TASK_ODOM_PERIOD_MS);

        // === Velocity estimation ===
        Enc_est.update(vl_rpm_, vr_rpm_, dt_);
        vlong_est_ = Enc_est.get_linear_velocity();
        w_est_     = Enc_est.get_angular_velocity();

        pwm_l_ = pwm_motors.pwm_L;
        pwm_r_ = pwm_motors.pwm_R;

#if VERBOSE_ODOM
        Serial.print("vl_rpm vr_rpm: ");
        Serial.print(vl_rpm_);
        Serial.print(" ");
        Serial.println(vr_rpm_);
        Serial.print("vlong w: ");
        Serial.print(vlong_est_);
        Serial.print(" ");
        Serial.println(w_est_);
#endif

        // Feedback queue
        float feedback_data[2] = {vl_rpm_, vr_rpm_};
        xQueueOverwrite(feedback_q_, &feedback_data);

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
        auto chk_f32 = [&](float v) {
            uint8_t* p = reinterpret_cast<uint8_t*>(&v);
            for (int i = 0; i < 4; i++) checksum ^= p[i];
        };

#if SEND_ODOM
        // Header
        chk_u8(HDR1); send_u8(HDR1);
        chk_u8(HDR2); send_u8(HDR2);

        // Type + LEN
        chk_u8(ODOM_ID);            send_u8(ODOM_ID);
        chk_u8(ODOM_PAYLOAD_LEN);   send_u8(ODOM_PAYLOAD_LEN);

        // Payload
        chk_u32(ID_); send_u32(ID_);

        chk_f32(vlong_est_); send_f32(vlong_est_);
        chk_f32(w_est_);     send_f32(w_est_);

        chk_f32(vl_rpm_); send_f32(vl_rpm_);
        chk_f32(vr_rpm_); send_f32(vr_rpm_);

        chk_u32(pwm_l_); send_u32(pwm_l_);
        chk_u32(pwm_r_); send_u32(pwm_r_);

        // Checksum finale (1 byte)
        send_u8(checksum);
#endif

        ID_++;
    }
};

