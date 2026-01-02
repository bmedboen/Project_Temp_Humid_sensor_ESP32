// time_manager.h
#pragma once

#include <Arduino.h>

/**
 * @brief Initializes and synchronizes the system time.
 * Priority: 
 * 1. External RTC (DS3231) - Most accurate.
 * 2. Internal ESP32 Clock - If valid (survived Deep Sleep).
 * 3. NTP (WiFi) - Fallback if hardware clocks are invalid.
 * * @return true if valid time is set, false otherwise.
 */
bool TimeManager_startAndSync();

/**
 * @brief Master function to set the system time.
 * Updates the internal ESP32 clock and conditionally updates the External RTC.
 * * @param t The time structure containing the new time.
 * @param updateRTC If true, the time is also written to the External RTC (DS3231).
 * Set this to 'false' if the time originated FROM the RTC (to prevent drift).
 * Default is 'true' (e.g. for Web/NTP updates).
 */
void TimeManager_setTime(struct tm t, bool updateRTC = true);

/**
 * @brief Get current time as a formatted String (YYYY-MM-DD HH:MM:SS).
 */
String getFormattedTime();

/**
 * @brief Checks if the current system time is valid (Year > 2024).
 */
bool TimeManager_isTimeSet();
