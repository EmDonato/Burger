#pragma once

#include <Arduino.h>
#include <stdint.h>

/**
 * @file encoder.h
 * @brief Quadrature wheel encoder interface.
 *
 * Handles encoder ticks, direction detection and RPM computation.
 */
class Encoder {
public:
    /**
     * @brief Constructor.
     * @param _pinA GPIO pin for encoder channel A.
     * @param _pinB GPIO pin for encoder channel B.
     * @param _ppr  Pulses per revolution of the encoder.
     * @param _gearRatio Gear ratio between motor and encoder (default 1.0).
     */
    Encoder(int _pinA, int _pinB, float _ppr, float _gearRatio = 1.0f);

    /**
     * @brief Initialize encoder pins and attach ISR.
     * @param isr Interrupt service routine for channel A.
     */
    void begin(void (*isr)());

    /**
     * @brief ISR handler for quadrature decoding.
     */
    void handleInterrupt();

    /**
     * @brief Compute output shaft RPM and reset tick counter.
     * @param dt_ms Time window in milliseconds.
     * @return Output RPM.
     */
    float getOutputRPMandReset(unsigned long dt_ms);

    /**
     * @brief Compute motor RPM (with gear ratio) and reset tick counter.
     * @param dt_ms Time window in milliseconds.
     * @return Motor RPM.
     */
    float getMotorRPMandReset(unsigned long dt_ms);

    /**
     * @brief Get last detected rotation direction.
     * @return +1 forward, -1 reverse, 0 stationary.
     */
    int getDirection() const;

private:
    volatile long tickCount;   ///< Encoder tick counter
    volatile int  direction;   ///< Rotation direction

    const int   pinA;
    const int   pinB;
    const float ppr;
    const float gearRatio;
};
