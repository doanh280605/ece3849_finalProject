#pragma once

#include "app_types.h"

void ControllerLogic_Update(
    const WirelessCommand &cmd,
    const SensorData &sensor,
    CarState &state);
