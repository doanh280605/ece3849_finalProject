#include "wireless.h"

#include <stddef.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_memmap.h"
}

#include "system_tasks.h"

static bool ParseJoystickByte(char byte, WirelessCommand *cmd);
static bool ParsePacketByte(char byte, WirelessCommand *cmd);
static bool ParsePacket(const char *packet, uint8_t length, WirelessCommand *cmd);
static MotionCommand ParseMotion(char motionByte, bool *ok);
static uint8_t ParseSpeedPercent(const char *text, uint8_t *index);
static DriveMode ParseMode(char modeByte);

static bool gPacketInProgress = false;

void Wireless_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {
    }

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(
        UART0_BASE,
        gSystemClockHz,
        9600U,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
}

bool Wireless_GetLatestCommand(WirelessCommand *cmd)
{
    if (cmd == NULL) {
        return false;
    }

    bool receivedCommand = false;
    WirelessCommand latest = {};

    while (UARTCharsAvail(UART0_BASE)) {
        const char byte = (char)UARTCharGetNonBlocking(UART0_BASE);

        if (ParsePacketByte(byte, cmd) ||
            (!gPacketInProgress && ParseJoystickByte(byte, cmd))) {
            latest = *cmd;
            receivedCommand = true;
        }
    }

    if (receivedCommand) {
        *cmd = latest;
        cmd->receivedTick = xTaskGetTickCount();
    }

    return receivedCommand;
}

static bool ParseJoystickByte(char byte, WirelessCommand *cmd)
{
    cmd->mode = MANUAL_MODE;
    cmd->speedPercent = 80U;

    switch (byte) {
        case 'U':
        case 'u':
        case 'F':
        case 'f':
            cmd->motion = CMD_FORWARD;
            break;
        case 'D':
        case 'd':
        case 'B':
        case 'b':
            cmd->motion = CMD_REVERSE;
            break;
        case 'L':
        case 'l':
            cmd->motion = CMD_LEFT;
            break;
        case 'R':
        case 'r':
        case 'X':
        case 'x':
            cmd->motion = CMD_RIGHT;
            break;
        case 'S':
        case 's':
        case '0':
        case 'C':
        case 'c':
        case 'N':
        case 'n':
            cmd->motion = CMD_STOP;
            cmd->speedPercent = 0U;
            break;
        default:
            return false;
    }

    cmd->valid = true;
    return true;
}

static bool ParsePacketByte(char byte, WirelessCommand *cmd)
{
    static char packet[16];
    static uint8_t length = 0U;
    if (byte == '<') {
        gPacketInProgress = true;
        length = 0U;
        return false;
    }

    if (!gPacketInProgress) {
        return false;
    }

    if (byte == '>') {
        gPacketInProgress = false;
        packet[length] = '\0';
        return ParsePacket(packet, length, cmd);
    }

    if (length >= (sizeof(packet) - 1U)) {
        gPacketInProgress = false;
        length = 0U;
        return false;
    }

    packet[length] = byte;
    length++;
    return false;
}

static bool ParsePacket(const char *packet, uint8_t length, WirelessCommand *cmd)
{
    bool motionOk = false;
    uint8_t index = 0U;

    if ((packet == NULL) || (cmd == NULL) || (length < 1U)) {
        return false;
    }

    cmd->motion = ParseMotion(packet[index], &motionOk);
    if (!motionOk) {
        return false;
    }
    index++;

    cmd->speedPercent = 80U;
    cmd->mode = MANUAL_MODE;

    if ((index < length) && (packet[index] == ',')) {
        index++;
        cmd->speedPercent = ParseSpeedPercent(packet, &index);
    }

    if ((index < length) && (packet[index] == ',')) {
        index++;
        if (index < length) {
            cmd->mode = ParseMode(packet[index]);
        }
    }

    if (cmd->motion == CMD_STOP) {
        cmd->speedPercent = 0U;
    }

    cmd->valid = true;
    return true;
}

static MotionCommand ParseMotion(char motionByte, bool *ok)
{
    *ok = true;

    switch (motionByte) {
        case 'U':
        case 'u':
        case 'F':
        case 'f':
            return CMD_FORWARD;
        case 'D':
        case 'd':
        case 'B':
        case 'b':
            return CMD_REVERSE;
        case 'L':
        case 'l':
            return CMD_LEFT;
        case 'R':
        case 'r':
        case 'X':
        case 'x':
            return CMD_RIGHT;
        case 'S':
        case 's':
        case '0':
        case 'C':
        case 'c':
        case 'N':
        case 'n':
            return CMD_STOP;
        default:
            *ok = false;
            return CMD_STOP;
    }
}

static uint8_t ParseSpeedPercent(const char *text, uint8_t *index)
{
    uint16_t value = 0U;

    while ((text[*index] >= '0') && (text[*index] <= '9')) {
        value = (uint16_t)((value * 10U) + (uint16_t)(text[*index] - '0'));
        (*index)++;
    }

    if (value > 100U) {
        value = 100U;
    }

    return (uint8_t)value;
}

static DriveMode ParseMode(char modeByte)
{
    if (modeByte == '1') {
        return AUTO_AVOID_MODE;
    }

    return MANUAL_MODE;
}
