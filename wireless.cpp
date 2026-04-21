#include "wireless.h"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

void Wireless_Init(void)
{
    // later: UART init for HC-05
}

bool Wireless_GetLatestCommand(WirelessCommand *cmd)
{
    if (cmd == NULL) {
        return false;
    }

    Wireless_MockInput(cmd);
    cmd->receivedTick = xTaskGetTickCount();
    return true;
}

void Wireless_MockInput(WirelessCommand *cmd)
{
    static uint32_t phase = 0U;

    if (cmd == NULL) {
        return;
    }

    phase++;

    cmd->valid = true;
    cmd->mode = MANUAL_MODE;
    cmd->speedPercent = 50U;

    if (phase < 200U) {
        cmd->motion = CMD_FORWARD;
    } else if (phase < 300U) {
        cmd->motion = CMD_LEFT;
    } else if (phase < 400U) {
        cmd->motion = CMD_RIGHT;
    } else {
        cmd->motion = CMD_STOP;
    }

    if (phase > 500U) {
        phase = 0U;
    }
}
