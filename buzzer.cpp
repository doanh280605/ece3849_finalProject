#include "buzzer.h"

extern "C" {
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"

#include "FreeRTOS.h"
#include "task.h"
}

static uint32_t gBuzzerSystemClock = 0;

static void Buzzer_Start(uint32_t frequency);
static void Buzzer_Stop(void);

void Buzzer_Init(uint32_t systemClock)
{
    gBuzzerSystemClock = systemClock;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {
    }

    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenDisable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);
}

void Buzzer_Play(uint32_t frequency, uint32_t durationMs)
{
    Buzzer_Start(frequency);
    vTaskDelay(pdMS_TO_TICKS(durationMs));
    Buzzer_Stop();
}

static void Buzzer_Start(uint32_t frequency)
{
    uint32_t load;

    if (frequency == 0U) {
        Buzzer_Stop();
        return;
    }

    load = gBuzzerSystemClock / frequency;
    if (load < 2U) {
        load = 2U;
    }

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, load);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, load / 2U);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);
}

static void Buzzer_Stop(void)
{
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false);
    PWMGenDisable(PWM0_BASE, PWM_GEN_0);
}
