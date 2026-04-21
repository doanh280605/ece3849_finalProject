#include "system_tasks.h"

#include "app_config.h"
#include "controller_logic.h"
#include "debug_uart.h"
#include "motor.h"
#include "safety.h"
#include "sensor.h"
#include "wireless.h"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

uint32_t gSystemClockHz = 0U;
AppContext gAppContext = {};

static void SensorTask(void *pvParameters);
static void WirelessTask(void *pvParameters);
static void SafetyTask(void *pvParameters);
static void ControlTask(void *pvParameters);
static void MotorTask(void *pvParameters);
static void DebugTask(void *pvParameters);
static void UpdateMotorStateCache(
    MotorState &motorState,
    MotionCommand motion,
    uint8_t speedPercent);

void SystemTasks_InitContext(void)
{
    gAppContext.wirelessCommand = {CMD_STOP, 0U, MANUAL_MODE, false, 0U, 0U};
    gAppContext.sensor = {100.0f, false, true, true, 0.0f};
    gAppContext.car = {MANUAL_MODE, CMD_STOP, CMD_STOP, 0U, 0, false, false, false};
    gAppContext.motor = {0, 0, false};
    gAppContext.safety = {true, false, false};
    gAppContext.lock = xSemaphoreCreateMutex();
}

bool SystemTasks_Create(void)
{
    if (gAppContext.lock == NULL) {
        return false;
    }

    const BaseType_t sensorOk = xTaskCreate(
        SensorTask, "Sensor", kDefaultTaskStackWords, NULL, kSensorTaskPriority, NULL);
    const BaseType_t wirelessOk = xTaskCreate(
        WirelessTask, "Wireless", kDefaultTaskStackWords, NULL, kWirelessTaskPriority, NULL);
    const BaseType_t safetyOk = xTaskCreate(
        SafetyTask, "Safety", kDefaultTaskStackWords, NULL, kSafetyTaskPriority, NULL);
    const BaseType_t controlOk = xTaskCreate(
        ControlTask, "Control", kDefaultTaskStackWords, NULL, kControlTaskPriority, NULL);
    const BaseType_t motorOk = xTaskCreate(
        MotorTask, "Motor", kDefaultTaskStackWords, NULL, kControlTaskPriority, NULL);
    const BaseType_t debugOk = xTaskCreate(
        DebugTask, "Debug", kDefaultTaskStackWords, NULL, kDebugTaskPriority, NULL);

    return (sensorOk == pdPASS) &&
           (wirelessOk == pdPASS) &&
           (safetyOk == pdPASS) &&
           (controlOk == pdPASS) &&
           (motorOk == pdPASS) &&
           (debugOk == pdPASS);
}

static void SensorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        Sensor_Read(&gAppContext.sensor);
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kSensorTaskPeriodMs));
    }
}

static void WirelessTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        if (Wireless_GetLatestCommand(&gAppContext.wirelessCommand)) {
            gAppContext.wirelessCommand.sequenceNumber++;
            gAppContext.car.signalAlive = true;
        } else {
            gAppContext.wirelessCommand.valid = false;
            gAppContext.car.signalAlive = false;
        }
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kWirelessTaskPeriodMs));
    }
}

static void SafetyTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        gAppContext.safety = Safety_Evaluate(
            gAppContext.wirelessCommand,
            gAppContext.sensor,
            gAppContext.car);
        if (gAppContext.safety.signalTimeout ||
            gAppContext.safety.sensorFault ||
            gAppContext.safety.obstacleStop) {
            gAppContext.car.mode = SAFE_STOP_MODE;
            gAppContext.car.finalMotion = CMD_STOP;
            gAppContext.car.speedPercent = 0U;
            gAppContext.car.safetyStopActive = true;
            UpdateMotorStateCache(gAppContext.motor, CMD_STOP, 0U);
        }
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kSafetyTaskPeriodMs));
    }
}

static void ControlTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        gAppContext.car.signalAlive = !gAppContext.safety.signalTimeout;
        if (gAppContext.safety.signalTimeout || gAppContext.safety.sensorFault) {
            gAppContext.car.signalAlive = false;
        }
        ControllerLogic_Update(
            gAppContext.wirelessCommand,
            gAppContext.sensor,
            gAppContext.car);
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kControlTaskPeriodMs));
    }
}

static void MotorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        if (gAppContext.car.safetyStopActive ||
            gAppContext.car.finalMotion == CMD_STOP ||
            gAppContext.car.finalMotion == CMD_BRAKE) {
            UpdateMotorStateCache(gAppContext.motor, CMD_STOP, 0U);
            Motor_Stop();
        } else {
            UpdateMotorStateCache(
                gAppContext.motor,
                gAppContext.car.finalMotion,
                gAppContext.car.speedPercent);
            Motor_SetCommand(
                gAppContext.car.finalMotion,
                gAppContext.car.speedPercent);
        }
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kControlTaskPeriodMs));
    }
}

static void DebugTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        DebugUart_LogState(gAppContext);
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kDebugTaskPeriodMs));
    }
}

static void UpdateMotorStateCache(
    MotorState &motorState,
    MotionCommand motion,
    uint8_t speedPercent)
{
    if (speedPercent > 100U) {
        speedPercent = 100U;
    }

    switch (motion) {
        case CMD_FORWARD:
            motorState.leftPwmPercent = (int16_t)speedPercent;
            motorState.rightPwmPercent = (int16_t)speedPercent;
            motorState.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_REVERSE:
            motorState.leftPwmPercent = -(int16_t)speedPercent;
            motorState.rightPwmPercent = -(int16_t)speedPercent;
            motorState.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_LEFT:
            motorState.leftPwmPercent = (int16_t)(speedPercent / 2U);
            motorState.rightPwmPercent = (int16_t)speedPercent;
            motorState.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_RIGHT:
            motorState.leftPwmPercent = (int16_t)speedPercent;
            motorState.rightPwmPercent = (int16_t)(speedPercent / 2U);
            motorState.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_BRAKE:
        case CMD_STOP:
        default:
            motorState.leftPwmPercent = 0;
            motorState.rightPwmPercent = 0;
            motorState.driverEnabled = false;
            break;
    }
}
