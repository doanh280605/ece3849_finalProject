#include "sensor.h"

void Sensor_Init(void)
{
    // later: ultrasonic trigger/echo init
}

void Sensor_Read(SensorData *data)
{
    if (data == NULL) {
        return;
    }

    data->distanceCm = 100.0f;
    data->obstacleDetected = false;
    data->ultrasonicHealthy = true;
    data->imuHealthy = true;
    data->yawRateDps = 0.0f;
}

void Sensor_MockRead(SensorData *data)
{
    static uint32_t tick = 0U;

    if (data == NULL) {
        return;
    }

    tick++;

    data->distanceCm = 100.0f;
    data->obstacleDetected = false;
    data->ultrasonicHealthy = true;
    data->imuHealthy = true;
    data->yawRateDps = 0.0f;

    if ((tick > 365U) && (tick < 410U)) {
        data->distanceCm = 10.0f;
        data->obstacleDetected = true;
    }

    if ((tick > 455U) && (tick < 500U)) {
        data->distanceCm = 12.0f;
        data->obstacleDetected = true;
    }

    if (tick > 620U) {
        tick = 0U;
    }
}
