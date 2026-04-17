// Shared application-level objects used across modules.
// Keep this header minimal to avoid tight coupling.

#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "grlib/grlib.h"
}

#include <stdint.h>

// Global graphics context for the LCD driver
extern tContext gContext;

// System clock frequency (Hz) set during startup
extern uint32_t gSysClk;

// Protects the shared game state accessed by multiple tasks.
extern SemaphoreHandle_t gGameStateMutex;

// declares the shared event group handle so multiple files can use it
extern EventGroupHandle_t xGameEvents;

#define EVT_FOOD_EATEN  (1 << 0) // defines the bit for "food eaten", bit 0 means that event happened
#define EVT_GAME_OVER   (1 << 1) // bit 1 means game over
