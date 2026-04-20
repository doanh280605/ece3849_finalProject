#include "controller_logic.h"

#include "motor.h"

void ControllerLogic_RunCycle(AppContext &appContext)
{
    appContext.car.mode = appContext.wirelessCommand.mode;
    appContext.car.commandedMotion = appContext.wirelessCommand.motion;
    appContext.car.speedPercent = appContext.wirelessCommand.speedPercent;
    appContext.car.signalAlive = !appContext.safety.signalTimeout;
    appContext.car.safetyStopActive =
        appContext.safety.signalTimeout ||
        appContext.safety.sensorFault ||
        appContext.safety.obstacleStop;
    appContext.car.brakeActive = (appContext.car.commandedMotion == CMD_BRAKE);

    if (appContext.car.safetyStopActive) {
        appContext.car.mode = SAFE_STOP_MODE;
        appContext.car.finalMotion = CMD_STOP;
        appContext.car.speedPercent = 0U;
        Motor_Stop(appContext.motor);
        return;
    }

    appContext.car.finalMotion = appContext.car.commandedMotion;
    Motor_Apply(appContext.motor, appContext.car.finalMotion, appContext.car.speedPercent);
}
