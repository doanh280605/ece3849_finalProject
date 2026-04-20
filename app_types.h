#ifndef APP_TYPES_H
#define APP_TYPES_H

extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

#include <stdbool.h>
#include <stdint.h>

enum DriveMode {
    MANUAL_MODE = 0,
    AUTO_AVOID_MODE,
    SAFE_STOP_MODE
};

enum MotionCommand {
    CMD_STOP = 0,
    CMD_FORWARD,
    CMD_REVERSE,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_BRAKE
};

struct WirelessCommand {
    MotionCommand motion;
    uint8_t speedPercent;
    DriveMode mode;
    bool valid;
    uint32_t sequenceNumber;
    uint32_t receivedTick;
};

struct SensorData {
    float distanceCm;
    bool obstacleDetected;
    bool ultrasonicHealthy;
    bool imuHealthy;
    float yawRateDps;
};

struct CarState {
    DriveMode mode;
    MotionCommand commandedMotion;
    MotionCommand finalMotion;
    uint8_t speedPercent;
    int8_t steeringPercent;
    bool signalAlive;
    bool safetyStopActive;
    bool brakeActive;
};

struct MotorState {
    int16_t leftPwmPercent;
    int16_t rightPwmPercent;
    bool driverEnabled;
};

struct SafetyState {
    bool signalTimeout;
    bool obstacleStop;
    bool sensorFault;
};

struct AppContext {
    WirelessCommand wirelessCommand;
    SensorData sensor;
    CarState car;
    MotorState motor;
    SafetyState safety;
    SemaphoreHandle_t lock;
};

#endif
