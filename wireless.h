#pragma once

#include "app_types.h"

void Wireless_Init(void);
void Wireless_Poll(WirelessCommand &wirelessCommand, CarState &carState);
