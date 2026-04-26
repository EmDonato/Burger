#include "WSerial.h"
#include "encoder.h"

// =============================================================
// === Constructor ===
// =============================================================
Encoder::Encoder(int pinA, int pinB, float ppr, float gearRatio)
    : tickCount(0),
      direction(0),
      pinA(pinA),
      pinB(pinB),
      ppr(ppr),
      gearRatio(gearRatio)
{
}

// =============================================================
// === Initialization ===
// =============================================================
void Encoder::begin(void (*isr)()) {
    pinMode(pinA, INPUT);
    pinMode(pinB, INPUT);

    attachInterrupt(digitalPinToInterrupt(pinA), isr, CHANGE);
}

// =============================================================
// === ISR handler ===
// =============================================================
void Encoder::handleInterrupt() {
    int a = digitalRead(pinA);
    int b = digitalRead(pinB);
    if (a == b) {
        tickCount++;
        direction = +1;
    } else {
        tickCount--;
        direction = -1;
    }
}

// =============================================================
// === RPM computation ===
// =============================================================
float Encoder::getOutputRPMandReset(unsigned long dt_ms) {
    noInterrupts();
    long ticks = tickCount;
    tickCount = 0;
    interrupts();

    if (dt_ms == 0) {
        return 0.0f;
    }

    return (ticks * 60000.0f) / (ppr * dt_ms);
}

float Encoder::getMotorRPMandReset(unsigned long dt_ms) {
    return getOutputRPMandReset(dt_ms) * gearRatio;
}

// =============================================================
// === Accessors ===
// =============================================================
int Encoder::getDirection() const {
    return direction;
}

