#include "controller_logic.h"

void ControllerLogic_Update(
    const WirelessCommand &cmd,
    const SensorData &sensor,
    CarState &state)
{
    state.mode = cmd.mode;
    state.commandedMotion = cmd.motion;
    state.finalMotion = cmd.motion;
    state.speedPercent = cmd.speedPercent;
    state.brakeActive = (cmd.motion == CMD_BRAKE);
    state.safetyStopActive = false;

    if (!state.signalAlive) {
        state.mode = SAFE_STOP_MODE;
        state.finalMotion = CMD_STOP;
        state.speedPercent = 0U;
        state.brakeActive = false;
        state.safetyStopActive = true;
        return;
    }

    if (sensor.obstacleDetected && (cmd.motion == CMD_FORWARD)) {
        state.finalMotion = CMD_STOP;
        state.speedPercent = 0U;
        state.brakeActive = false;
        state.safetyStopActive = true;
        return;
    }

    if (state.mode == AUTO_AVOID_MODE && sensor.obstacleDetected) {
        state.finalMotion = CMD_LEFT;
        state.speedPercent = (cmd.speedPercent > 40U) ? 40U : cmd.speedPercent;
        state.brakeActive = false;
        state.safetyStopActive = true;
        return;
    }

    if (cmd.motion == CMD_BRAKE) {
        state.finalMotion = CMD_BRAKE;
        state.speedPercent = 0U;
        state.brakeActive = true;
    }
}
