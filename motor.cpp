#include "motor.h"

#include "app_config.h"

void Motor_Init(void)
{
}

void Motor_Stop(MotorState &motorState)
{
    motorState.leftPwmPercent = 0;
    motorState.rightPwmPercent = 0;
    motorState.driverEnabled = false;
}

void Motor_Apply(MotorState &motorState, MotionCommand motion, uint8_t speedPercent)
{
    if (speedPercent > 100U) {
        speedPercent = 100U;
    }

    switch (motion) {
        case CMD_FORWARD:
            motorState.leftPwmPercent = (int16_t)speedPercent;
            motorState.rightPwmPercent = (int16_t)speedPercent;
            break;
        case CMD_REVERSE:
            motorState.leftPwmPercent = -(int16_t)speedPercent;
            motorState.rightPwmPercent = -(int16_t)speedPercent;
            break;
        case CMD_LEFT:
            motorState.leftPwmPercent = (int16_t)(speedPercent / 2U);
            motorState.rightPwmPercent = (int16_t)speedPercent;
            break;
        case CMD_RIGHT:
            motorState.leftPwmPercent = (int16_t)speedPercent;
            motorState.rightPwmPercent = (int16_t)(speedPercent / 2U);
            break;
        case CMD_BRAKE:
        case CMD_STOP:
        default:
            motorState.leftPwmPercent = 0;
            motorState.rightPwmPercent = 0;
            break;
    }

    motorState.driverEnabled = (motion != CMD_STOP) && (motion != CMD_BRAKE) && (speedPercent > 0U);
}
