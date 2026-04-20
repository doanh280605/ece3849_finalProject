#pragma once

#include "app_types.h"

void Safety_Init(void);
SafetyState Safety_Evaluate(
    const WirelessCommand &wirelessCommand,
    const SensorData &sensorData,
    const CarState &carState);
