#pragma once
#include <Arduino.h>

class PID {
public:
    PID(float Kp, float Ki, float Kd,
        float dt,
        float integratorMin, float integratorMax,
        int outputMin, int outputMax);

    int compute(float setpoint, float measurement);

    void reset();

    void setTunings(float Kp, float Ki, float Kd);
    void setInterval(float dt);

    void setWindupLimits(float integratorMin, float integratorMax);
    void setOutputLimits(int outputMin, int outputMax);

    void setDeadZone(int dz);

    void setDerivativeFilterAlpha(float alpha);

    void setFeedforwardParams(float a, float b);
    void enableFeedforward(bool enable);

    bool isSaturated() const;

private:
    // Gains
    float _Kp, _Ki, _Kd;

    // Timing
    float _dt;

    // States
    float _integrator;
    float _prevMeas;
    float _dFiltered;

    // Limits
    float _intMin, _intMax;
    int _outMin, _outMax;

    // Dead-zone
    int _deadZone;

    // Derivative filter
    float _dFilterAlpha;

    // Feedforward
    float _ffA, _ffB;
    bool _useFeedforward;

public:
    // Debug / telemetry
    float error = 0.0f;
    int output = 0;
};

