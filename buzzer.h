#pragma once

#include <stdint.h>

void Buzzer_Init(uint32_t systemClock);
void Buzzer_Play(uint32_t frequency, uint32_t durationMs);
