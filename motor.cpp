#include "motor.h"

#include <stdint.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
}

static const uint8_t kAin1Pin = GPIO_PIN_0;  // PL0 -> TB6612 AIN1
static const uint8_t kAin2Pin = GPIO_PIN_1;  // PL1 -> TB6612 AIN2
static const uint8_t kPwmaPin = GPIO_PIN_2;  // PL2 -> TB6612 PWMA software PWM
static const uint8_t kMotorPins = kAin1Pin | kAin2Pin | kPwmaPin;
static const uint8_t kSoftwarePwmPeriodTicks = 20U;
static const TickType_t kDirectionChangeStopDelayTicks = pdMS_TO_TICKS(100U);

static int8_t gCurrentDirection = 0;
static uint8_t gPwmPhase = 0U;

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

    GPIOPinWrite(GPIO_PORTL_BASE, kMotorPins, value);
}

void Motor_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) {
    }

    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, kMotorPins);
    Motor_Stop();
}

void Motor_SetCommand(MotionCommand cmd, uint8_t speedPercent)
{
    int8_t requestedDirection = 0;

    if (speedPercent > 100U) {
        speedPercent = 100U;
    }

    if (speedPercent == 0U) {
        Motor_Stop();
        return;
    }

    switch (cmd) {
        case CMD_FORWARD:
            requestedDirection = 1;
            break;
        case CMD_REVERSE:
            requestedDirection = -1;
            break;
        case CMD_BRAKE:
            Motor_WriteA(true, true, true);
            gCurrentDirection = 0;
            break;
        case CMD_LEFT:
        case CMD_RIGHT:
            // One-motor bring-up: sideways joystick motion should not drive forward.
            Motor_Stop();
            break;
        case CMD_STOP:
        default:
            Motor_Stop();
            break;
    }

    if (requestedDirection == 0) {
        return;
    }

    if ((gCurrentDirection != 0) && (gCurrentDirection != requestedDirection)) {
        Motor_Stop();
        vTaskDelay(kDirectionChangeStopDelayTicks);
    }

    gCurrentDirection = requestedDirection;

    gPwmPhase++;
    if (gPwmPhase >= kSoftwarePwmPeriodTicks) {
        gPwmPhase = 0U;
    }

    const uint8_t highTicks =
        (uint8_t)((((uint16_t)speedPercent * kSoftwarePwmPeriodTicks) + 99U) / 100U);
    const bool pwmHigh = (gPwmPhase < highTicks);

    if (requestedDirection > 0) {
        Motor_WriteA(true, false, pwmHigh);
    } else {
        Motor_WriteA(false, true, pwmHigh);
    }
}

void Motor_Stop(void)
{
    gCurrentDirection = 0;
    gPwmPhase = 0U;
    Motor_WriteA(false, false, true);
}
