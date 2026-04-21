#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#include "app_types.h"

void Motor_Init(void);
void Motor_SetCommand(MotionCommand cmd, uint8_t speedPercent);
void Motor_Stop(void);

#endif
