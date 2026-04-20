#include "debug_uart.h"

#include <stddef.h>

void DebugUart_Init(uint32_t systemClockHz)
{
    (void)systemClockHz;
}

void DebugUart_Log(const char *message)
{
    (void)message;
}

void DebugUart_LogState(const AppContext &appContext)
{
    (void)appContext;
}
