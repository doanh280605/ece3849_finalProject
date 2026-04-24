#include "motor.h"

#include <stdint.h>

extern "C" {
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
}

static const uint8_t kAin1Pin = GPIO_PIN_0;  // PN0 -> TB6612 AIN1
static const uint8_t kAin2Pin = GPIO_PIN_1;  // PN1 -> TB6612 AIN2
static const uint8_t kPwmaPin = GPIO_PIN_2;  // PN2 -> TB6612 PWMA as GPIO enable
static const uint8_t kMotorPins = kAin1Pin | kAin2Pin | kPwmaPin;

static void Motor_WriteA(bool ain1High, bool ain2High, bool pwmaHigh)
{
    uint8_t value = 0U;

    if (ain1High) {
        value |= kAin1Pin;
    }
    if (ain2High) {
        value |= kAin2Pin;
    }
    if (pwmaHigh) {
        value |= kPwmaPin;
    }

    GPIOPinWrite(GPIO_PORTN_BASE, kMotorPins, value);
}

void Motor_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {
    }

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, kMotorPins);
    Motor_WriteA(false, false, false);
}

void Motor_SetCommand(MotionCommand cmd, uint8_t speedPercent)
{
    const bool motorEnabled = (speedPercent > 0U);

    switch (cmd) {
        case CMD_FORWARD:
            Motor_WriteA(true, false, motorEnabled);
            break;
        case CMD_REVERSE:
            Motor_WriteA(false, true, motorEnabled);
            break;
        case CMD_BRAKE:
            Motor_WriteA(true, true, motorEnabled);
            break;
        case CMD_LEFT:
        case CMD_RIGHT:
            // Single-motor GPIO bring-up: treat turns as forward until PWMB is added.
            Motor_WriteA(true, false, motorEnabled);
            break;
        case CMD_STOP:
        default:
            Motor_WriteA(false, false, false);
            break;
    }
}

void Motor_Stop(void)
{
    Motor_SetCommand(CMD_STOP, 0U);
}
