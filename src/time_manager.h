// time_manager.h

#pragma once
#include <Arduino.h> // For String type and millis()

/**
 * @brief Initializes time. Logic: Internal -> External RTC -> NTP.
 * Updates External RTC if NTP is used.
 * @return true if valid time is set, false otherwise.
 */
bool TimeManager_startAndSync();

String getFormattedTime();

bool TimeManager_isTimeSet();
