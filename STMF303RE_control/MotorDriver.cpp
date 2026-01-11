#include "MotorDriver.h"

L298N::L298N(HardwareTimer *timer,
             uint32_t chA,
             uint32_t chB)
    : tim(timer),
      channelA(chA),
      channelB(chB)
{
}

void L298N::init(int in1, int in2, int in3, int in4)
{
    _in1 = in1; _in2 = in2;
    _in3 = in3; _in4 = in4;

    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    pinMode(_in3, OUTPUT);
    pinMode(_in4, OUTPUT);

    stopAll();
}

void L298N::forward(int duty, int motorID)
{
    if (!isValidMotorID(motorID)) return;
    duty = clampDuty(duty);

    if (motorID == 0) {
        digitalWrite(_in1, HIGH);
        digitalWrite(_in2, LOW);
        tim->setCaptureCompare(channelA, duty, PERCENT_COMPARE_FORMAT);
    } else {
        digitalWrite(_in3, HIGH);
        digitalWrite(_in4, LOW);
        tim->setCaptureCompare(channelB, duty, PERCENT_COMPARE_FORMAT);
    }
}

void L298N::backward(int duty, int motorID)
{
    if (!isValidMotorID(motorID)) return;
    duty = clampDuty(duty);

    if (motorID == 0) {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, HIGH);
        tim->setCaptureCompare(channelA, duty, PERCENT_COMPARE_FORMAT);
    } else {
        digitalWrite(_in3, LOW);
        digitalWrite(_in4, HIGH);
        tim->setCaptureCompare(channelB, duty, PERCENT_COMPARE_FORMAT);
    }
}

void L298N::stop(int motorID)
{
    if (!isValidMotorID(motorID)) return;

    if (motorID == 0) {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, LOW);
        tim->setCaptureCompare(channelA, 0, PERCENT_COMPARE_FORMAT);
    } else {
        digitalWrite(_in3, LOW);
        digitalWrite(_in4, LOW);
        tim->setCaptureCompare(channelB, 0, PERCENT_COMPARE_FORMAT);
    }
}

void L298N::stopAll()
{
    stop(0);
    stop(1);
}

bool L298N::isValidMotorID(int id)
{
    return (id == 0 || id == 1);
}

int L298N::clampDuty(int duty)
{
    if (duty < 0)   return 0;
    if (duty > 100) return 100;
    return duty;
}
