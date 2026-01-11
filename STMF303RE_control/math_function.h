#pragma once

#include "control_config.h"
#include <cmath>
#include <Arduino.h>

/**
 * @brief Wheel speeds container (no STL, embedded-safe)
 */
struct WheelSpeeds {
    float left;
    float right;
};

/**
 * @brief Reference message (longitudinal + angular velocity)
 */
// Feedback encoder → control task
struct WheelRpmFeedback {
    float vl_rpm;
    float vr_rpm;
};

// Reference → control task
struct RefMsg {
    float v_ref;
    float w_ref;
};
/**
 * @brief Computes wheel angular velocities from normalized commands.
 *
 * @param v_norm Normalized longitudinal velocity [-1, 1]
 * @param w_norm Normalized angular velocity [-1, 1]
 * @return Wheel angular velocities [rad/s]
 */
inline WheelSpeeds compute_wheel_speeds(float v_norm, float w_norm)
{
    if (std::abs(v_norm) < 1e-2f) {
        w_norm *= 0.8f;
    }

    float v = constrain(v_norm, -1.0f, 1.0f) * V_MAX;
    float w = constrain(w_norm, -1.0f, 1.0f) * W_MAX;

    float v_right = v + (WHEEL_SEPARATION * 0.5f) * w;
    float v_left  = v - (WHEEL_SEPARATION * 0.5f) * w;

    float max_speed = std::max(std::abs(v_right), std::abs(v_left));
    if (max_speed > V_MAX) {
        float scale = V_MAX / max_speed;
        v_right *= scale;
        v_left  *= scale;
    }

    WheelSpeeds ws;
    ws.right = v_right / WHEEL_RADIUS;
    ws.left  = v_left  / WHEEL_RADIUS;

    return ws;
}

/**
 * @brief Converts wheel angular velocity to RPM.
 */
inline WheelSpeeds computeRPM(float omega_left, float omega_right)
{
    constexpr float factor = 60.0f / (2.0f * M_PI);

    WheelSpeeds rpm;
    rpm.left  = omega_left  * factor;
    rpm.right = omega_right * factor;

    return rpm;
}

/**
 * @brief Normalize angle to [-pi, pi]
 */
inline float normalize_angle(float angle)
{
    while (angle >  M_PI) angle -= 2.0f * M_PI;
    while (angle < -M_PI) angle += 2.0f * M_PI;
    return angle;
}

/**
 * @brief Sign function
 */
inline int sign(float x)
{
    return (x > 0.0f) - (x < 0.0f);
}

/**
 * @brief Complementary filter between encoder and IMU yaw
 */
inline float complementaryFilter(float theta_enc_raw, float theta_imu)
{
    constexpr float alpha = 0.98f;
    return alpha * theta_imu + (1.0f - alpha) * theta_enc_raw;
}

