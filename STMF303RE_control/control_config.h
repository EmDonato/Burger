// =================== control_config.h ===================
#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H
#include <cmath>


//MSG code sensor
#define IMU_ID 0x01
#define ODOM_ID 0x02
#define CMD_VEL_ID 0x03
#define ARM_ID 0x04
#define HB_ID 0x05
#define STRING_ID 0x06





// Covariance settings for odometry
#define MAX_SAMPLES     50    /**< Maximum samples for calibration or filtering */
#define PID_DT          100.0f    /**< PID control loop period in ms */
#define DEFAULT_DT      0.01f     /**< Default time step in seconds */
#define INT_MIN_PID     -50.0f  /**< PID integrator minimum limit */
#define INT_MAX_PID      50.0f  /**< PID integrator maximum limit */

// ==========================
// === LED AND SERVO PINS ===
// ==========================
//
// These pins provide simple visual debugging (LEDs)
// and control for a servo (rudder).
//
//micro users leds
// --- PIN DEFINITIONS ---

//#define LED_GREEN       PA5

// Encoder Left 
#define ENCODER_PINA_L  PA10   // Arduino pin 34
#define ENCODER_PINB_L  PA11   // Arduino pin 35

// Encoder Right 
#define ENCODER_PINA_R  PA0   // Arduino pin 16
#define ENCODER_PINB_R  PA1   // Arduino pin 17

// PWM pins


#define PWM_PINA        PB0   // Arduino pin 32 (TIM2_CH2)
#define PWM_PINB        PB1   // Arduino pin 14 (TIM3_CH2)

// Motor driver direction pins
#define IN_PINA         PB10   // Arduino pin 33
#define IN_PINB         PA6   // Arduino pin 25
#define IN_PINC         PB6   // Arduino pin 26
#define IN_PIND         PA7   // Arduino pin 27

#define SDA_PIN             PB9   /**< I2C SDA pin for IMU */
#define SCL_PIN             PB8   /**< I2C SCL pin for IMU */

// --- PWM LEDC SETTINGS ---

#define PWM_FREQ_HZ   1000   // 1 kHz
#define PWM_DUTY_1    50     // 50%
#define PWM_DUTY_2    25     // 25%

// ==========================
// === SYSTEM DEFINITIONS ===
// ==========================
#define MAX_TASKS 5            // number of RTOS tasks monitored by the TimeManager           




#define TICK_TIME_1S  pdMS_TO_TICKS(1000)
#define TICK_TIME_2S  pdMS_TO_TICKS(2000)
#define TICK_TIME_3S  pdMS_TO_TICKS(3000)
#define TICK_TIME_4S  pdMS_TO_TICKS(4000)
#define TICK_TIME_5S  pdMS_TO_TICKS(5000)

// ==========================
//      === PARAM ===
// ==========================
//

#define IMU_READ_DATA 100 //Hz
#define MOTOR_FEEDBACK_FREQ_MS 100 //ms
#define STATUS_MOTOR_FREQ_MS 500//ms
// ==========================
//   === CONTROL PARAM ===
// ==========================

#define KI 3 //25.0f                    /**< PID I gain */
#define KP 1 //24.0f                       /**< PID P gain */
#define KD  0.0f                          /**< PID D gain */

// ==========================
//   === MOTORS PARAM ===
// ==========================
//
// Motor and encoder parameters
#define ENCODER_PPR     (497 * 2) /**< Pulses per revolution of Hall encoder */
#define WHEEL_RADIUS    0.0346f    /**< Wheel radius in meters */
#define WHEEL_SEPARATION 0.20f    /**< Distance between wheels in meters */
    // Robot kinematics constants
#define V_MAX            0.362f               ///< Max linear speed [m/s]
#define W_MAX            2.0f * V_MAX / WHEEL_SEPARATION ///< Max angular speed [rad/s]
#define OUT_MIN         0         /**< Minimum PWM output */
#define OUT_MAX         100      /**< Maximum PWM output */
//#define DEADZONE        2048      /**< PWM deadzone threshold */


// ==========================
//   === RADIO CMD FILTER ===
// ==========================
//

#define TAU 0.05f
#define Dt  0.05f
#define TAU_MOTOR 0.1f
#define Dt_MOTOR  0.05f
#define ALPHA  0.9 //(TAU / (TAU + Dt))  // Low-pass filter coefficient //RUDDER
#define ALPHA_MOTOR  0.95 //(TAU_MOTOR / (TAU_MOTOR + Dt_MOTOR))  // Low-pass filter coefficient //MOTOR


// ============================
// == SAFETY FLAGS FOR FAULT ==
// ============================
//

#define BLOCK_SAFETY   1
#define CONNECTION_SAFETY   1



#endif