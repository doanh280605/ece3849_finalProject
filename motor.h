#pragma once

#include "app_types.h"

void Motor_Init(void);
void Motor_Stop(MotorState &motorState);
void Motor_Apply(MotorState &motorState, MotionCommand motion, uint8_t speedPercent);
