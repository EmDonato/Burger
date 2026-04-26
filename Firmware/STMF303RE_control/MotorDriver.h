#pragma once
#include <Arduino.h>
#include "STM32_PWM.h"
#include "control_config.h"

class L298N
{
public:
    L298N(HardwareTimer *timer,
          uint32_t chA,
          uint32_t chB);

    void init(int in1, int in2, int in3, int in4);

    void forward(int duty, int motorID);
    void backward(int duty, int motorID);
    void stop(int motorID);
    void stopAll();

private:
    HardwareTimer *tim;
    uint32_t channelA;
    uint32_t channelB;

    int _in1, _in2, _in3, _in4;

    bool isValidMotorID(int id);
    int clampDuty(int duty);
};
