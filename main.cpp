#include <stdbool.h>
#include <stdint.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "FreeRTOS.h"
#include "task.h"
}

#include "app_config.h"
#include "debug_uart.h"
#include "motor.h"
#include "safety.h"
#include "sensor.h"
#include "system_tasks.h"
#include "wireless.h"

static void ConfigureSystemClock(void);

int main(void)
{
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    ConfigureSystemClock();

    DebugUART_Init();
    Sensor_Init();
    Motor_Init();
    Wireless_Init();
    Safety_Init();
    Tasks_Create();

    IntMasterEnable();
    vTaskStartScheduler();

    while (true) {
    }
}

static void ConfigureSystemClock(void)
{
    gSystemClockHz = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        kSystemClockHz);
}
