#include <STM32FreeRTOS.h>
#include <Wire.h>

#include "IMU.hpp"
#include "freertos_config_tasks.h"
#include "control_config.h"
#include "ICM_20948.h"
#include "stm32f3xx.h"
#include "encoder.h"
#include "ISR.hpp"
#include "state.hpp"
#include "MotorDriver.h"
#include "PID.h"
#include "odometry.hpp"
#include "odometry_class.hpp"
#include "serialReciveTask.hpp"
#include "control.hpp"
#include "utility.hpp"



// ==========================================================
// === TASK SPECIFICATIONS ===
// ==========================================================

TaskSpec ImuSpec      = { TASK_IMU_NAME,     TASK_IMU_STACK,     TASK_IMU_PRIORITY,     TASK_IMU_PERIOD_MS };
TaskSpec OdomSpec     = { TASK_ODOM_NAME,    TASK_ODOM_STACK,    TASK_ODOM_PRIORITY,    TASK_ODOM_PERIOD_MS };
TaskSpec reciverSpec  = { TASK_RX_NAME,      TASK_RX_STACK,      TASK_RX_PRIORITY,      TASK_RX_PERIOD_MS };
TaskSpec controlSpec  = { TASK_CONTROL_NAME, TASK_CONTROL_STACK, TASK_CONTROL_PRIORITY, TASK_CONTROL_PERIOD_MS };
TaskSpec hbSpec       = { TASK_HB_NAME,      TASK_HB_STACK,      TASK_HB_PRIORITY,      TASK_HB_PERIOD_MS };

// ==========================================================
// === GLOBAL OBJECTS ===
// ==========================================================

 uint32_t state = 0;


// IMU
ICM_20948_I2C imu;
actualPWM pwm_motors;

// === PWM ===
HardwareTimer *MyTim = nullptr;
uint32_t channel1;
uint32_t channel2;

// === Motor driver (pointer, costruito in setup) ===
L298N *motor = nullptr;

// PID controllers
PID pid_L(KP, KI, KD, TASK_CONTROL_PERIOD_MS / 1000.0f,
          INT_MIN_PID, INT_MAX_PID, OUT_MIN, OUT_MAX);
PID pid_R(KP, KI, KD, TASK_CONTROL_PERIOD_MS / 1000.0f,
          INT_MIN_PID, INT_MAX_PID, OUT_MIN, OUT_MAX);

// Odometry
//UnicycleOdometry odometry(WHEEL_RADIUS, WHEEL_SEPARATION);
EncoderPoseEstimator Enc_est(WHEEL_RADIUS, WHEEL_SEPARATION);

// Timers + Motor driver (PWM)
// HardwareTimer timer1(TIM1);   // es. PA8
// HardwareTimer timer2(TIM2);   // es. PA0
// L298N motors(&timer1, TIM_CHANNEL_1,
//              &timer2, TIM_CHANNEL_1);

Encoder encoder_L(ENCODER_PINA_L, ENCODER_PINB_L, ENCODER_PPR, 46.8f);
Encoder encoder_R(ENCODER_PINA_R, ENCODER_PINB_R, ENCODER_PPR, 46.8f);

// Queues
QueueHandle_t reference_queue;
QueueHandle_t feedback_queue;
QueueHandle_t control_queues[2];

// ==========================================================
// === SETUP ===
// ==========================================================
void setup()
{
    Serial.begin(115200);
    while (!Serial);

    sendStringPacket(STRING_ID,"[INFO] setup start");

    // ----------------------------
    // === GPIO DIR ===
    // ----------------------------
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);



    pinMode(IN_PINA, OUTPUT);
    pinMode(IN_PINB, OUTPUT);
    pinMode(IN_PINC, OUTPUT);
    pinMode(IN_PIND, OUTPUT);


    digitalWrite(IN_PINA, HIGH);
    digitalWrite(IN_PINB, LOW);
    digitalWrite(IN_PINC, HIGH);
    digitalWrite(IN_PIND, LOW);
    // ----------------------------
    // === PWM PIN MODE ===
    // ----------------------------
    pinMode(PWM_PINA, OUTPUT);
    pinMode(PWM_PINB, OUTPUT);

    // ----------------------------
    // === PWM TIMER INIT ===
    // ----------------------------
    PinName pinName1 = digitalPinToPinName(PWM_PINA);
    TIM_TypeDef *Instance =
        (TIM_TypeDef *) pinmap_peripheral(pinName1, PinMap_PWM);

    if (Instance == nullptr) {
        sendStringPacket(STRING_ID,"[ERROR] PWM_PINA not hardware PWM");
        while (1);
    }

    channel1 = STM_PIN_CHANNEL(
        pinmap_function(pinName1, PinMap_PWM)
    );

    PinName pinName2 = digitalPinToPinName(PWM_PINB);
    channel2 = STM_PIN_CHANNEL(
        pinmap_function(pinName2, PinMap_PWM)
    );

    static HardwareTimer timer(Instance);  
    MyTim = &timer;

    MyTim->setPWM(channel1, PWM_PINA, PWM_FREQ_HZ, PWM_DUTY_1);
    MyTim->setPWM(channel2, PWM_PINB, PWM_FREQ_HZ, PWM_DUTY_2);

    sendStringPacket(STRING_ID,"[INFO] PWM initialized");

    // ----------------------------
    // === MOTOR DRIVER INIT ===
    // ----------------------------
    static L298N motor_driver(MyTim, channel1, channel2);  
    motor = &motor_driver;

    motor->init(IN_PINA, IN_PINB, IN_PINC, IN_PIND);

    sendStringPacket(STRING_ID,"[INFO] Motor driver ready");

    // ----------------------------
    // === TEST ===
    // ----------------------------
    motor->forward(0, 0);
    motor->forward(0, 1);
    delay(1000);

    motor->stopAll();
    sendStringPacket(STRING_ID,"[INFO] test OK");

    // ----------------------------
    // === IMU INIT ===
    // ----------------------------
    Wire.setSCL(SCL_PIN);
    Wire.setSDA(SDA_PIN);
    Wire.begin();
    Wire.setClock(400000);
    delay(200);

    imu.begin(Wire, 1);
    while (imu.status != ICM_20948_Stat_Ok) {}

    configureIMU();
    delay(200);
    calibrateGyro();
    sendStringPacket(STRING_ID,"[INFO] IMU initialized");
    // Attach encoder interrupt service routines
    encoder_L.begin(encoderISR_L);
    encoder_R.begin(encoderISR_R);
 
    // ----------------------------
    // === PID INIT ===
    // ----------------------------
    pid_L.reset();
    pid_R.reset();

    pid_L.setDeadZone(0.01);
    pid_R.setDeadZone(0.01);

    // pid_L.setFeedforwardParams(20.88f, 500.85f);
    // pid_R.setFeedforwardParams(20.88f, 500.85f);

    // pid_L.enableFeedforward(1);
    // pid_R.enableFeedforward(1);
    sendStringPacket(STRING_ID,"[INFO] PID initialized");

    // ----------------------------
    // === FREERTOS QUEUES ===
    // ----------------------------
    reference_queue = xQueueCreate(1, sizeof(float) * 2);
    feedback_queue  = xQueueCreate(1, sizeof(float) * 2);

    control_queues[0] = reference_queue;
    control_queues[1] = feedback_queue;

    // ----------------------------
    // === TASK OBJECTS ===
    // ----------------------------
    static SerialReceiveTask referenceReceiver(reciverSpec, reference_queue);
    static ImuTask imuTask(ImuSpec);
    static controlTask controller(controlSpec, control_queues);
    static OdometryTask odomTask(OdomSpec, feedback_queue);
    static HBTask HBTask(hbSpec);

    // ----------------------------
    // === START TASKS ===
    // ----------------------------
    
    referenceReceiver.Begin();
    controller.Begin();
    odomTask.Begin();
    imuTask.Begin();
    HBTask.Begin();
    sendStringPacket(STRING_ID,"[INFO] FreeRTOS starting");

    // ----------------------------
    // === START SCHEDULER ===
    // ----------------------------
    digitalWrite(LED_GREEN, LOW);

    // ----------------------------
    // === FreeRTOS START ===
    // ----------------------------
    vTaskStartScheduler();

    while (1);
}

// ==========================================================
// === LOOP ===
// ==========================================================
// Unused: FreeRTOS runs instead
void loop() {
  //Serial.println("in loop");
}

