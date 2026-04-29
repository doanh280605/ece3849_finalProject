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
static const uint8_t kMotorAPins = kAin1Pin | kAin2Pin | kPwmaPin;
static const uint8_t kBin1Pin = GPIO_PIN_1;  // PF1 -> TB6612 BIN1
static const uint8_t kBin2Pin = GPIO_PIN_2;  // PF2 -> TB6612 BIN2
static const uint8_t kPwmbPin = GPIO_PIN_3;  // PF3 -> TB6612 PWMB software PWM
static const uint8_t kMotorBPins = kBin1Pin | kBin2Pin | kPwmbPin;
static const uint8_t kSoftwarePwmPeriodTicks = 20U;
static const TickType_t kDirectionChangeStopDelayTicks = pdMS_TO_TICKS(100U);

static int8_t gCurrentLeftDirection = 0;
static int8_t gCurrentRightDirection = 0;
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

    GPIOPinWrite(GPIO_PORTL_BASE, kMotorAPins, value);
}

static void Motor_WriteB(bool bin1High, bool bin2High, bool pwmbHigh)
{
    uint8_t value = 0U;

    if (bin1High) {
        value |= kBin1Pin;
    }
    if (bin2High) {
        value |= kBin2Pin;
    }
    if (pwmbHigh) {
        value |= kPwmbPin;
    }

    GPIOPinWrite(GPIO_PORTF_BASE, kMotorBPins, value);
}

static bool Motor_NeedsDirectionStop(int8_t currentDirection, int8_t requestedDirection)
{
    return (currentDirection != 0) &&
           (requestedDirection != 0) &&
           (currentDirection != requestedDirection);
}

static void Motor_WriteChannelA(int8_t direction, bool pwmHigh)
{
    if (direction > 0) {
        Motor_WriteA(true, false, pwmHigh);
    } else if (direction < 0) {
        Motor_WriteA(false, true, pwmHigh);
    } else {
        Motor_WriteA(false, false, true);
    }
}

static void Motor_WriteChannelB(int8_t direction, bool pwmHigh)
{
    if (direction > 0) {
        Motor_WriteB(true, false, pwmHigh);
    } else if (direction < 0) {
        Motor_WriteB(false, true, pwmHigh);
    } else {
        Motor_WriteB(false, false, true);
    }
}

void Motor_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {
    }

    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, kMotorAPins);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, kMotorBPins);
    Motor_Stop();
}

void Motor_SetCommand(MotionCommand cmd, uint8_t speedPercent)
{
    int8_t requestedLeftDirection = 0;
    int8_t requestedRightDirection = 0;

    if (speedPercent > 100U) {
        speedPercent = 100U;
    }

    if (speedPercent == 0U) {
        Motor_Stop();
        return;
    }

    switch (cmd) {
        case CMD_FORWARD:
            requestedLeftDirection = 1;
            requestedRightDirection = 1;
            break;
        case CMD_REVERSE:
            requestedLeftDirection = -1;
            requestedRightDirection = -1;
            break;
        case CMD_BRAKE:
            Motor_WriteA(true, true, true);
            Motor_WriteB(true, true, true);
            gCurrentLeftDirection = 0;
            gCurrentRightDirection = 0;
            break;
        case CMD_RIGHT:
            requestedLeftDirection = 1;
            requestedRightDirection = 0;
            break;
        case CMD_LEFT:
            requestedLeftDirection = 0;
            requestedRightDirection = 1;
            break;
        case CMD_STOP:
        default:
            Motor_Stop();
            break;
    }

    if ((requestedLeftDirection == 0) && (requestedRightDirection == 0)) {
        return;
    }

    if (Motor_NeedsDirectionStop(gCurrentLeftDirection, requestedLeftDirection) ||
        Motor_NeedsDirectionStop(gCurrentRightDirection, requestedRightDirection)) {
        Motor_Stop();
        vTaskDelay(kDirectionChangeStopDelayTicks);
    }

    gCurrentLeftDirection = requestedLeftDirection;
    gCurrentRightDirection = requestedRightDirection;

    gPwmPhase++;
    if (gPwmPhase >= kSoftwarePwmPeriodTicks) {
        gPwmPhase = 0U;
    }

    const uint8_t highTicks =
        (uint8_t)((((uint16_t)speedPercent * kSoftwarePwmPeriodTicks) + 99U) / 100U);
    const bool pwmHigh = (gPwmPhase < highTicks);

    Motor_WriteChannelA(requestedLeftDirection, pwmHigh);
    Motor_WriteChannelB(requestedRightDirection, pwmHigh);
}

void Motor_Stop(void)
{
    gCurrentLeftDirection = 0;
    gCurrentRightDirection = 0;
    gPwmPhase = 0U;
    Motor_WriteA(false, false, true);
    Motor_WriteB(false, false, true);
}
