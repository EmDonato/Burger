#include "PID.h"
#include <cmath>

PID::PID(float Kp, float Ki, float Kd,
         float dt,
         float integratorMin, float integratorMax,
         int outputMin, int outputMax)
    : _Kp(Kp), _Ki(Ki), _Kd(Kd),
      _dt(dt > 1e-4f ? dt : 1e-3f),
      _integrator(0.0f),
      _prevMeas(0.0f),
      _dFiltered(0.0f),
      _intMin(integratorMin), _intMax(integratorMax),
      _outMin(outputMin), _outMax(outputMax),
      _deadZone(0),
      _dFilterAlpha(0.1f),
      _ffA(0.0f), _ffB(0.0f),
      _useFeedforward(false)
{
}

int PID::compute(float setpoint, float measurement)
{
    error = setpoint - measurement;

    // === Proportional ===
    float P = _Kp * error;

    // === Derivative (on measurement) ===
    float D = 0.0f;
    float dMeas = (measurement - _prevMeas) / _dt;
    _dFiltered = _dFilterAlpha * dMeas
               + (1.0f - _dFilterAlpha) * _dFiltered;
    D = -_Kd * _dFiltered;

    // === Feedforward ===
    float FF = 0.0f;
    if (_useFeedforward && std::abs(setpoint) > 1e-3f) {
        FF = _ffA * setpoint + _ffB;
    }

    // === Anti-windup: tentative integrator ===
    float I_next = _integrator + _Ki * error * _dt;

    float u = P + I_next + D + FF;

    // === Saturation ===
    int raw = static_cast<int>(std::round(u));

    if (raw > 0)      raw += _deadZone;
    else if (raw < 0) raw -= _deadZone;

    output = constrain(raw, _outMin, _outMax);

    // === Integrator update only if NOT saturated ===
    if (output > _outMin && output < _outMax) {
        _integrator = I_next;
        _integrator = constrain(_integrator, _intMin, _intMax);
    }

    _prevMeas = measurement;
    return output;
}

void PID::reset()
{
    _integrator = 0.0f;
    _prevMeas = 0.0f;
    _dFiltered = 0.0f;
    error = 0.0f;
    output = 0;
}

void PID::setTunings(float Kp, float Ki, float Kd)
{
    _Kp = Kp;
    _Ki = Ki;
    _Kd = Kd;
}

void PID::setInterval(float dt)
{
    _dt = (dt > 1e-4f) ? dt : 1e-3f;
}

void PID::setWindupLimits(float integratorMin, float integratorMax)
{
    _intMin = integratorMin;
    _intMax = integratorMax;
}

void PID::setOutputLimits(int outputMin, int outputMax)
{
    _outMin = outputMin;
    _outMax = outputMax;
}

void PID::setDeadZone(int dz)
{
    _deadZone = abs(dz);
}

void PID::setDerivativeFilterAlpha(float alpha)
{
    _dFilterAlpha = constrain(alpha, 0.0f, 1.0f);
}

void PID::setFeedforwardParams(float a, float b)
{
    _ffA = a;
    _ffB = b;
}

void PID::enableFeedforward(bool enable)
{
    _useFeedforward = enable;
}

bool PID::isSaturated() const
{
    return (output == _outMin || output == _outMax);
}

