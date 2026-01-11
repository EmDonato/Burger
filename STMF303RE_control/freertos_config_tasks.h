/**
 * @file freertos_config_tasks.h
 * @brief FreeRTOS task configuration parameters.
 *
 * This file defines task specifications (stack size, priority, period)
 * used for scheduling and timing across the embedded system.
 *
 * Each task uses the following TaskSpec structure:
 *     TaskSpec { const char* name, uint16_t stackSize, uint8_t priority, uint16_t period_ms }
 */

#ifndef FREERTOS_CONFIG_TASKS_H
#define FREERTOS_CONFIG_TASKS_H


// =============================
// === Task Configuration ======
// =============================

// IMU Task – Fast sensor acquisition (~66.7 Hz)
#define TASK_IMU_NAME           "ImuTask"
#define TASK_IMU_STACK          1024
#define TASK_IMU_PRIORITY       9
#define TASK_IMU_PERIOD_MS      15 //15

// Logging Task – Periodic system logging (~50 Hz)
#define TASK_ODOM_NAME           "OdomTask"
#define TASK_ODOM_STACK          2048
#define TASK_ODOM_PRIORITY       2
#define TASK_ODOM_PERIOD_MS      50

// Motor Control Task – Motor driver loop (~10 Hz)
#define TASK_CONTROL_NAME       "MotorDriverTack"
#define TASK_CONTROL_STACK      1024
#define TASK_CONTROL_PRIORITY   5
#define TASK_CONTROL_PERIOD_MS  25


// Optional: Serial Debug Task – low-frequency output (~3.3 Hz)
#define TASK_RX_NAME        "SerialDebug"
#define TASK_RX_STACK       1024
#define TASK_RX_PRIORITY    7
#define TASK_RX_PERIOD_MS   20


// Optional: Serial Debug Task – low-frequency output (~3.3 Hz)
#define TASK_HB_NAME        "heartbeat"
#define TASK_HB_STACK       1024
#define TASK_HB_PRIORITY    1
#define TASK_HB_PERIOD_MS   100


#endif // FREERTOS_CONFIG_TASKS_H
