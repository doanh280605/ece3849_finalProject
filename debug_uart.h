#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "app_types.h"

void DebugUART_Init(void);
void DebugUART_Print(const char *msg);
void DebugUART_PrintState(const AppContext *appContext);
void DebugUART_PrintSafetyEvent(const AppContext *appContext);

#endif
