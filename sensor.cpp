#include "sensor.h"

void Sensor_Init(void)
{
    // later: ultrasonic trigger/echo init
}

void Sensor_Read(SensorData *data)
{
    Sensor_MockRead(data);
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

    if ((tick > 250U) && (tick < 320U)) {
        data->distanceCm = 10.0f;
        data->obstacleDetected = true;
    }

    if (tick > 500U) {
        tick = 0U;
    }
}
