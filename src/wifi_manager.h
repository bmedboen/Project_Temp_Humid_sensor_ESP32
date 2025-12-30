// wifi_manager.h

#pragma once

#include <WiFi.h>

bool wifi_manager_init_AP();
bool wifi_manager_connect_STA();
void wifi_manager_turnOff();
