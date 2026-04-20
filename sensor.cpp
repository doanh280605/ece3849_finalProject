#include "sensor.h"

void Sensor_Init(void)
{
}

SensorData Sensor_Read(void)
{
    SensorData sample = {};
    sample.distanceCm = 100.0f;
    sample.obstacleDetected = false;
    sample.ultrasonicHealthy = true;
    sample.imuHealthy = true;
    sample.yawRateDps = 0.0f;
    return sample;
}
