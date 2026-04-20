#include "wireless.h"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

void Wireless_Init(void)
{
}

void Wireless_Poll(WirelessCommand &wirelessCommand, CarState &carState)
{
    wirelessCommand.valid = true;
    wirelessCommand.motion = CMD_STOP;
    wirelessCommand.speedPercent = 0U;
    wirelessCommand.mode = carState.mode;
    wirelessCommand.sequenceNumber++;
    wirelessCommand.receivedTick = xTaskGetTickCount();
    carState.signalAlive = true;
}
