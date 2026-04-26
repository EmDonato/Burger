#pragma once

#include <utility>

/**
 * @class EncoderPoseEstimator
 * @brief Estimates robot linear and angular velocity using wheel encoders.
 */
class EncoderPoseEstimator {
public:
    /**
     * @brief Constructor.
     * @param wheel_radius Radius of the wheels [m]
     * @param wheel_separation Distance between wheels [m]
     */
    EncoderPoseEstimator(float wheel_radius, float wheel_separation);

    /**
     * @brief Update velocity estimates from wheel RPMs.
     * @param rpm_left  Left wheel RPM
     * @param rpm_right Right wheel RPM
     * @param dt        Sampling time [s]
     */
    void update(float rpm_left, float rpm_right, float dt);

    /// @return Linear velocity [m/s]
    float get_linear_velocity() const;

    /// @return Angular velocity [rad/s]
    float get_angular_velocity() const;

    /// @return Pair {v_left, v_right} [m/s]
    std::pair<float, float> get_wheels_velocity() const;

private:
    float R_;      // Wheel radius [m]
    float L_;      // Wheel separation [m]

    float v_;      // Linear velocity [m/s]
    float omega_;  // Angular velocity [rad/s]

    float v_l_;    // Left wheel linear velocity [m/s]
    float v_r_;    // Right wheel linear velocity [m/s]
};
