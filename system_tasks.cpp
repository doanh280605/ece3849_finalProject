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
static void DebugTask(void *pvParameters);

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
    const BaseType_t debugOk = xTaskCreate(
        DebugTask, "Debug", kDefaultTaskStackWords, NULL, kDebugTaskPriority, NULL);

    return (sensorOk == pdPASS) &&
           (wirelessOk == pdPASS) &&
           (safetyOk == pdPASS) &&
           (controlOk == pdPASS) &&
           (debugOk == pdPASS);
}

static void SensorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        gAppContext.sensor = Sensor_Read();
        xSemaphoreGive(gAppContext.lock);
        vTaskDelay(pdMS_TO_TICKS(kSensorTaskPeriodMs));
    }
}

static void WirelessTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        xSemaphoreTake(gAppContext.lock, portMAX_DELAY);
        Wireless_Poll(gAppContext.wirelessCommand, gAppContext.car);
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
            Motor_Stop(gAppContext.motor);
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
        ControllerLogic_RunCycle(gAppContext);
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
