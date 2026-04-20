#include "safety.h"

#include "app_config.h"

void Safety_Init(void)
{
}

SafetyState Safety_Evaluate(
    const WirelessCommand &wirelessCommand,
    const SensorData &sensorData,
    const CarState &carState)
{
    SafetyState safety = {};
    safety.signalTimeout = !wirelessCommand.valid;
    safety.sensorFault = !sensorData.ultrasonicHealthy || !sensorData.imuHealthy;
    safety.obstacleStop = (carState.mode != MANUAL_MODE) && sensorData.obstacleDetected;
    return safety;
}
