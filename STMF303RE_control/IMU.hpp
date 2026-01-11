#pragma once

#include "Task.hpp"
#include <Arduino.h>
#include "freertos_config_tasks.h"
#include "ICM_20948.h"
#include "control_config.h"

// =============================================================
// === IMU HANDLE ===
// =============================================================
extern ICM_20948_I2C imu;

// =============================================================
// === CONFIGURATION MACROS ===
// =============================================================
#define VERBOSE_IMU 0
#define SEND_IMU    1


// payload = idMsg + 15 float (NO checksum)
static constexpr uint8_t IMU_PAYLOAD_LEN =
    sizeof(uint32_t) + 15 * sizeof(float);

// =============================================================
// === GYRO BIAS ===
// =============================================================
float bx = 0.0f;
float by = 0.0f;
float bz = 0.0f;

// =============================================================
// === IMU CONFIGURATION ===
// =============================================================
void configureIMU()
{
    imu.swReset();
    delay(250);

    imu.sleep(false);
    imu.lowPower(false);

    imu.setSampleMode(
        (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
        ICM_20948_Sample_Mode_Continuous
    );

    ICM_20948_fss_t fss;
    fss.a = gpm4;     // ±4 g
    fss.g = dps500;   // ±500 dps

    imu.setFullScale(
        (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
        fss
    );

    ICM_20948_dlpcfg_t dlpf;
    dlpf.a = acc_d23bw9_n34bw4;
    dlpf.g = gyr_d23bw9_n35bw9;

    imu.setDLPFcfg(
        (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
        dlpf
    );

    imu.enableDLPF(ICM_20948_Internal_Acc, true);
    imu.enableDLPF(ICM_20948_Internal_Gyr, true);

    imu.startupMagnetometer();
}

// =============================================================
// === GYRO CALIBRATION ===
// =============================================================
void calibrateGyro()
{
    sendStringPacket(STRING_ID, "Calibrating gyro... keep IMU still!");

    constexpr int N = 600;
    float sx = 0.0f, sy = 0.0f, sz = 0.0f;

    for (int i = 0; i < N; i++) {
        while (!imu.dataReady());
        imu.getAGMT();

        sx += imu.gyrX();
        sy += imu.gyrY();
        sz += imu.gyrZ();

        delay(3);
    }

    bx = (sx / N) * DEG_TO_RAD;
    by = (sy / N) * DEG_TO_RAD;
    bz = (sz / N) * DEG_TO_RAD;
}

// =============================================================
// === IMU TASK ===
// =============================================================
class ImuTask : public TaskBase
{
private:
    float ax_, ay_, az_;
    float gx_, gy_, gz_;
    float mx_, my_, mz_;
    float roll_, pitch_, yaw_;

    const float dt_ = TASK_IMU_PERIOD_MS / 1000.0f;
    uint32_t ID_ = 0;

public:
    ImuTask(const TaskSpec& s) : TaskBase(s) {}

protected:
    void setup() override
    {
        yaw_ = 0.0f;
        sendStringPacket(STRING_ID, "[IMU] imu task started");
    }

    bool wait() override
    {
        return true;
    }

    void loop() override
    {
        imu.getAGMT();

        // Accelerometer (g)
        ax_ = imu.accX() / 1000.0f;
        ay_ = imu.accY() / 1000.0f;
        az_ = imu.accZ() / 1000.0f;

        // Gyroscope (rad/s, bias removed)
        gx_ = imu.gyrX() * DEG_TO_RAD - bx;
        gy_ = imu.gyrY() * DEG_TO_RAD - by;
        gz_ = imu.gyrZ() * DEG_TO_RAD - bz;

        // Magnetometer (uT)
        mx_ = imu.magX();
        my_ = imu.magY();
        mz_ = imu.magZ();

        // Roll & pitch
        roll_  = atan2(ay_, az_);
        pitch_ = atan2(-ax_, sqrt(ay_ * ay_ + az_ * az_));

        // Yaw integration
        yaw_ += gz_ * dt_;

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

#if SEND_IMU
        // Header
        chk_u8(HDR1); send_u8(HDR1);
        chk_u8(HDR2); send_u8(HDR2);

        // Type + len
        chk_u8(IMU_ID);           send_u8(IMU_ID);
        chk_u8(IMU_PAYLOAD_LEN);  send_u8(IMU_PAYLOAD_LEN);

        // Payload
        chk_u32(ID_); send_u32(ID_);

        chk_f32(roll_);  send_f32(roll_);
        chk_f32(pitch_); send_f32(pitch_);
        chk_f32(yaw_);   send_f32(yaw_);

        chk_f32(ax_); send_f32(ax_);
        chk_f32(ay_); send_f32(ay_);
        chk_f32(az_); send_f32(az_);

        chk_f32(gx_); send_f32(gx_);
        chk_f32(gy_); send_f32(gy_);
        chk_f32(gz_); send_f32(gz_);

        chk_f32(mx_); send_f32(mx_);
        chk_f32(my_); send_f32(my_);
        chk_f32(mz_); send_f32(mz_);

        chk_f32(bx); send_f32(bx);
        chk_f32(by); send_f32(by);
        chk_f32(bz); send_f32(bz);

        // Checksum finale (1 byte, fuori dal payload)
        send_u8(checksum);
#endif

        ID_++;
    }
};
