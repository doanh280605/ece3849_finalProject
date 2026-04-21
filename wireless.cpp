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

    if (phase < 140U) {
        cmd->motion = CMD_FORWARD;
    } else if (phase < 200U) {
        cmd->motion = CMD_STOP;
    } else if (phase < 280U) {
        cmd->motion = CMD_LEFT;
    } else if (phase < 360U) {
        cmd->motion = CMD_RIGHT;
    } else if (phase < 430U) {
        cmd->motion = CMD_FORWARD;
    } else if (phase < 520U) {
        cmd->motion = CMD_FORWARD;
        cmd->mode = AUTO_AVOID_MODE;
        cmd->speedPercent = 60U;
    } else {
        cmd->motion = CMD_STOP;
    }

    if (phase > 620U) {
        phase = 0U;
    }
}
