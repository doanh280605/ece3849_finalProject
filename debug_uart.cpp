#include "debug_uart.h"

#include <stdio.h>

static const char *MotionCommandToString(MotionCommand motion);
static const char *DriveModeToString(DriveMode mode);

void DebugUART_Init(void)
{
}

void DebugUART_Print(const char *msg)
{
    if (msg == NULL) {
        return;
    }

    puts(msg);
}

void DebugUART_PrintState(const AppContext *appContext)
{
    char buffer[192];

    if (appContext == NULL) {
        return;
    }

    (void)snprintf(
        buffer,
        sizeof(buffer),
        "CMD rx=%s spd=%u mode=%s dist=%.1f obs=%d final=%s motor(L=%d,R=%d,en=%d) link(age=%lu,to=%d)",
        MotionCommandToString(appContext->wirelessCommand.motion),
        (unsigned int)appContext->wirelessCommand.speedPercent,
        DriveModeToString(appContext->car.mode),
        (double)appContext->sensor.distanceCm,
        appContext->sensor.obstacleDetected ? 1 : 0,
        MotionCommandToString(appContext->car.finalMotion),
        (int)appContext->motor.leftPwmPercent,
        (int)appContext->motor.rightPwmPercent,
        appContext->motor.driverEnabled ? 1 : 0,
        (unsigned long)appContext->safety.wirelessAgeMs,
        appContext->safety.signalTimeout ? 1 : 0);
    DebugUART_Print(buffer);
}

void DebugUART_PrintTransition(const AppContext *appContext)
{
    char buffer[160];

    if (appContext == NULL) {
        return;
    }

    (void)snprintf(
        buffer,
        sizeof(buffer),
        "STATE mode=%s cmd=%s final=%s safety=%d steer=%d",
        DriveModeToString(appContext->car.mode),
        MotionCommandToString(appContext->car.commandedMotion),
        MotionCommandToString(appContext->car.finalMotion),
        appContext->car.safetyStopActive ? 1 : 0,
        (int)appContext->car.steeringPercent);
    DebugUART_Print(buffer);
}

void DebugUART_PrintSafetyEvent(const AppContext *appContext)
{
    char buffer[160];

    if (appContext == NULL) {
        return;
    }

    (void)snprintf(
        buffer,
        sizeof(buffer),
        "SAFETY override timeout=%d obstacle=%d sensorFault=%d age=%lums final=%s",
        appContext->safety.signalTimeout ? 1 : 0,
        appContext->safety.obstacleStop ? 1 : 0,
        appContext->safety.sensorFault ? 1 : 0,
        (unsigned long)appContext->safety.wirelessAgeMs,
        MotionCommandToString(appContext->car.finalMotion));
    DebugUART_Print(buffer);
}

static const char *MotionCommandToString(MotionCommand motion)
{
    switch (motion) {
        case CMD_FORWARD:
            return "FWD";
        case CMD_REVERSE:
            return "REV";
        case CMD_LEFT:
            return "LEFT";
        case CMD_RIGHT:
            return "RIGHT";
        case CMD_BRAKE:
            return "BRAKE";
        case CMD_STOP:
        default:
            return "STOP";
    }
}

static const char *DriveModeToString(DriveMode mode)
{
    switch (mode) {
        case AUTO_AVOID_MODE:
            return "AUTO";
        case SAFE_STOP_MODE:
            return "SAFE";
        case MANUAL_MODE:
        default:
            return "MANUAL";
    }
}
