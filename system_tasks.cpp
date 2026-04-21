#include "system_tasks.h"

#include "app_config.h"
#include "app_types.h"
#include "controller_logic.h"
#include "debug_uart.h"
#include "motor.h"
#include "safety.h"
#include "sensor.h"
#include "wireless.h"

extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
}

uint32_t gSystemClockHz = 0U;

static WirelessCommand gCmd = {CMD_STOP, 0U, MANUAL_MODE, false, 0U, 0U};
static SensorData gSensor = {100.0f, false, true, true, 0.0f};
static CarState gState = {MANUAL_MODE, CMD_STOP, CMD_STOP, 0U, 0, false, false, false};
static MotorState gMotor = {0, 0, false};
static SafetyState gSafety = {true, false, false, 0U, 0U};
static TickType_t gLastWirelessTick = 0U;

static SemaphoreHandle_t gStateMutex = NULL;

static void WirelessRxTask(void *pvParameters);
static void SensorTask(void *pvParameters);
static void ControlTask(void *pvParameters);
static void MotorTask(void *pvParameters);
static void SafetyMonitorTask(void *pvParameters);
static void DebugTask(void *pvParameters);

static void InitializeSharedState(void);
static void UpdateMotorStateCache(MotionCommand motion, uint8_t speedPercent);
static void BuildSnapshot(AppContext *appContext);

static const uint32_t kWirelessPeriodMs = 20U;
static const uint32_t kSensorPeriodMs = 50U;
static const uint32_t kControlPeriodMs = 20U;
static const uint32_t kMotorPeriodMs = 20U;
static const uint32_t kSafetyPeriodMs = 100U;
static const uint32_t kSignalTimeoutMs = 250U;

void Tasks_Create(void)
{
    BaseType_t status = pdPASS;

    InitializeSharedState();
    gStateMutex = xSemaphoreCreateMutex();

    if (gStateMutex == NULL) {
        DebugUART_Print("ERROR: failed to create gStateMutex");
        while (1) {
        }
    }

    status &= xTaskCreate(
        WirelessRxTask,
        "WirelessRx",
        kDefaultTaskStackWords,
        NULL,
        kWirelessTaskPriority,
        NULL);
    status &= xTaskCreate(
        SensorTask,
        "Sensor",
        kDefaultTaskStackWords,
        NULL,
        kSensorTaskPriority,
        NULL);
    status &= xTaskCreate(
        ControlTask,
        "Control",
        kDefaultTaskStackWords,
        NULL,
        kControlTaskPriority,
        NULL);
    status &= xTaskCreate(
        MotorTask,
        "Motor",
        kDefaultTaskStackWords,
        NULL,
        kControlTaskPriority,
        NULL);
    status &= xTaskCreate(
        SafetyMonitorTask,
        "Safety",
        kDefaultTaskStackWords,
        NULL,
        kSafetyTaskPriority,
        NULL);
    status &= xTaskCreate(
        DebugTask,
        "Debug",
        kDefaultTaskStackWords,
        NULL,
        kDebugTaskPriority,
        NULL);

    if (status != pdPASS) {
        DebugUART_Print("ERROR: failed to create RTOS tasks");
        while (1) {
        }
    }
}

static void InitializeSharedState(void)
{
    gCmd.motion = CMD_STOP;
    gCmd.speedPercent = 0U;
    gCmd.mode = MANUAL_MODE;
    gCmd.valid = false;
    gCmd.sequenceNumber = 0U;
    gCmd.receivedTick = 0U;

    gSensor.distanceCm = 100.0f;
    gSensor.obstacleDetected = false;
    gSensor.ultrasonicHealthy = true;
    gSensor.imuHealthy = true;
    gSensor.yawRateDps = 0.0f;

    gState.mode = MANUAL_MODE;
    gState.commandedMotion = CMD_STOP;
    gState.finalMotion = CMD_STOP;
    gState.speedPercent = 0U;
    gState.steeringPercent = 0;
    gState.signalAlive = false;
    gState.safetyStopActive = false;
    gState.brakeActive = false;

    gMotor.leftPwmPercent = 0;
    gMotor.rightPwmPercent = 0;
    gMotor.driverEnabled = false;

    gSafety.signalTimeout = true;
    gSafety.obstacleStop = false;
    gSafety.sensorFault = false;
    gSafety.lastWirelessTick = 0U;
    gSafety.wirelessAgeMs = 0U;
    gLastWirelessTick = 0U;
}

static void WirelessRxTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        WirelessCommand temp = {};

        if (Wireless_GetLatestCommand(&temp)) {
            xSemaphoreTake(gStateMutex, portMAX_DELAY);
            temp.sequenceNumber = gCmd.sequenceNumber + 1U;
            gCmd = temp;
            gLastWirelessTick = xTaskGetTickCount();
            gCmd.receivedTick = (uint32_t)gLastWirelessTick;
            gSafety.lastWirelessTick = (uint32_t)gLastWirelessTick;
            gSafety.wirelessAgeMs = 0U;
            gState.signalAlive = true;
            xSemaphoreGive(gStateMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(kWirelessPeriodMs));
    }
}

static void SensorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        SensorData temp = {};
        Sensor_Read(&temp);

        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gSensor = temp;
        xSemaphoreGive(gStateMutex);

        vTaskDelay(pdMS_TO_TICKS(kSensorPeriodMs));
    }
}

static void ControlTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        ControllerLogic_Update(gCmd, gSensor, gState);
        xSemaphoreGive(gStateMutex);

        vTaskDelay(pdMS_TO_TICKS(kControlPeriodMs));
    }
}

static void MotorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        MotionCommand motion = CMD_STOP;
        uint8_t speedPercent = 0U;

        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        motion = gState.finalMotion;
        speedPercent = gState.speedPercent;
        UpdateMotorStateCache(motion, speedPercent);
        xSemaphoreGive(gStateMutex);

        if (motion == CMD_STOP || motion == CMD_BRAKE) {
            Motor_Stop();
        } else {
            Motor_SetCommand(motion, speedPercent);
        }

        vTaskDelay(pdMS_TO_TICKS(kMotorPeriodMs));
    }
}

static void SafetyMonitorTask(void *pvParameters)
{
    (void)pvParameters;
    bool previousOverride = false;

    for (;;) {
        AppContext snapshot = {};

        xSemaphoreTake(gStateMutex, portMAX_DELAY);

        if (gLastWirelessTick == 0U) {
            gSafety.signalTimeout = true;
            gSafety.lastWirelessTick = 0U;
            gSafety.wirelessAgeMs = kSignalTimeoutMs + kSafetyPeriodMs;
        } else {
            const TickType_t currentTick = xTaskGetTickCount();
            const TickType_t wirelessAgeTicks = currentTick - gLastWirelessTick;
            gSafety.lastWirelessTick = (uint32_t)gLastWirelessTick;
            gSafety.wirelessAgeMs = (uint32_t)((wirelessAgeTicks * 1000U) / configTICK_RATE_HZ);
            gSafety.signalTimeout = wirelessAgeTicks > pdMS_TO_TICKS(kSignalTimeoutMs);
        }

        gState.signalAlive = !gSafety.signalTimeout;
        gSafety.sensorFault = !gSensor.ultrasonicHealthy || !gSensor.imuHealthy;
        gSafety.obstacleStop = (gState.mode != MANUAL_MODE) && gSensor.obstacleDetected;

        if (gSafety.signalTimeout || gSafety.sensorFault || gSafety.obstacleStop) {
            gState.mode = SAFE_STOP_MODE;
            gState.finalMotion = CMD_STOP;
            gState.speedPercent = 0U;
            gState.safetyStopActive = true;
            gState.brakeActive = false;
            UpdateMotorStateCache(CMD_STOP, 0U);
        } else {
            gState.safetyStopActive = false;
        }

        if (!previousOverride && gState.safetyStopActive) {
            BuildSnapshot(&snapshot);
        }
        previousOverride = gState.safetyStopActive;

        xSemaphoreGive(gStateMutex);

        if (snapshot.lock == NULL && snapshot.car.safetyStopActive) {
            DebugUART_PrintSafetyEvent(&snapshot);
        }

        vTaskDelay(pdMS_TO_TICKS(kSafetyPeriodMs));
    }
}

static void DebugTask(void *pvParameters)
{
    (void)pvParameters;
    AppContext previousSnapshot = {};
    bool firstSample = true;

    for (;;) {
        AppContext snapshot = {};

        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        BuildSnapshot(&snapshot);
        xSemaphoreGive(gStateMutex);

        if (firstSample ||
            snapshot.car.mode != previousSnapshot.car.mode ||
            snapshot.car.commandedMotion != previousSnapshot.car.commandedMotion ||
            snapshot.car.finalMotion != previousSnapshot.car.finalMotion ||
            snapshot.car.safetyStopActive != previousSnapshot.car.safetyStopActive ||
            snapshot.sensor.obstacleDetected != previousSnapshot.sensor.obstacleDetected) {
            DebugUART_PrintTransition(&snapshot);
            previousSnapshot = snapshot;
            firstSample = false;
        }

        DebugUART_PrintState(&snapshot);
        vTaskDelay(pdMS_TO_TICKS(kDebugTaskPeriodMs));
    }
}

static void UpdateMotorStateCache(MotionCommand motion, uint8_t speedPercent)
{
    if (speedPercent > 100U) {
        speedPercent = 100U;
    }

    switch (motion) {
        case CMD_FORWARD:
            gMotor.leftPwmPercent = (int16_t)speedPercent;
            gMotor.rightPwmPercent = (int16_t)speedPercent;
            gMotor.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_REVERSE:
            gMotor.leftPwmPercent = -(int16_t)speedPercent;
            gMotor.rightPwmPercent = -(int16_t)speedPercent;
            gMotor.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_LEFT:
            gMotor.leftPwmPercent = (int16_t)(speedPercent / 2U);
            gMotor.rightPwmPercent = (int16_t)speedPercent;
            gMotor.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_RIGHT:
            gMotor.leftPwmPercent = (int16_t)speedPercent;
            gMotor.rightPwmPercent = (int16_t)(speedPercent / 2U);
            gMotor.driverEnabled = (speedPercent > 0U);
            break;
        case CMD_BRAKE:
        case CMD_STOP:
        default:
            gMotor.leftPwmPercent = 0;
            gMotor.rightPwmPercent = 0;
            gMotor.driverEnabled = false;
            break;
    }
}

static void BuildSnapshot(AppContext *appContext)
{
    if (appContext == NULL) {
        return;
    }

    appContext->wirelessCommand = gCmd;
    appContext->sensor = gSensor;
    appContext->car = gState;
    appContext->motor = gMotor;
    appContext->safety = gSafety;
    appContext->lock = NULL;
}
