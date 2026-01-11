#include "EncoderPoseEstimator.hpp"
#include <cmath>

EncoderPoseEstimator::EncoderPoseEstimator(float wheel_radius,
                                           float wheel_separation)
    : R_(wheel_radius),
      L_(wheel_separation),
      v_(0.0f),
      omega_(0.0f),
      v_l_(0.0f),
      v_r_(0.0f)
{
}

// Update velocities from wheel RPM
void EncoderPoseEstimator::update(float rpm_l, float rpm_r, float dt) {
    (void)dt; // dt not used here, kept for interface consistency

    // RPM -> rad/s -> linear velocity
    v_l_ = rpm_l * 2.0f * M_PI / 60.0f * R_;
    v_r_ = rpm_r * 2.0f * M_PI / 60.0f * R_;

    // Linear and angular velocity
    v_     = 0.5f * (v_l_ + v_r_);
    omega_ = (v_r_ - v_l_) / L_;
}

float EncoderPoseEstimator::get_linear_velocity() const {
    return v_;
}

float EncoderPoseEstimator::get_angular_velocity() const {
    return omega_;
}

std::pair<float, float> EncoderPoseEstimator::get_wheels_velocity() const {
    return {v_l_, v_r_};
}
