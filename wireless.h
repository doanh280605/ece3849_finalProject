#ifndef WIRELESS_H
#define WIRELESS_H

#include <stdbool.h>

#include "app_types.h"

void Wireless_Init(void);
bool Wireless_GetLatestCommand(WirelessCommand *cmd);
void Wireless_MockInput(WirelessCommand *cmd);

#endif
