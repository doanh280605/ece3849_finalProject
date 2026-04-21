#include "motor.h"

void Motor_Init(void)
{
    // later: GPIO + PWM init
}

void Motor_SetCommand(MotionCommand cmd, uint8_t speedPercent)
{
    (void)cmd;
    (void)speedPercent;
    // later: translate cmd into motor driver signals
}

void Motor_Stop(void)
{
    Motor_SetCommand(CMD_STOP, 0U);
}
