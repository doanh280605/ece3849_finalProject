#pragma once

#include "app_types.h"

extern uint32_t gSystemClockHz;
extern AppContext gAppContext;

void SystemTasks_InitContext(void);
bool SystemTasks_Create(void);
