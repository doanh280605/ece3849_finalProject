#pragma once

#include "app_types.h"

void DebugUart_Init(uint32_t systemClockHz);
void DebugUart_Log(const char *message);
void DebugUart_LogState(const AppContext &appContext);
