#include <stdbool.h>
#include <stdint.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"

#include "FreeRTOS.h"
#include "task.h"
}

#include "debug_uart.h"
#include "motor.h"
#include "sensor.h"
#include "system_tasks.h"
#include "wireless.h"

int main(void)
{
    FPUEnable();
    FPULazyStackingEnable();

    gSystemClockHz = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);

    DebugUART_Init();
    Motor_Init();
    Wireless_Init();
    Sensor_Init();

    Tasks_Create();

    vTaskStartScheduler();

    while (true) {
    }
}
