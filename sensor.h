#ifndef SENSOR_H
#define SENSOR_H

#include "app_types.h"

void Sensor_Init(void);
void Sensor_Read(SensorData *data);
void Sensor_MockRead(SensorData *data);

#endif
