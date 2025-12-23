// sleep_manager.h
#pragma once

#include <Arduino.h>

/**
 * @brief Calculates the remaining time to sleep based on the logging interval.
 * @return The calculated time in microseconds. Returns a negative value if overdue.
 */
int64_t calculateSleepTime(uint64_t timeLastLogged_ms, uint64_t currentTime_ms, uint64_t overhead_ms, uint64_t logInterval_ms);

/**
 * @brief Enters deep sleep safely.
 * Includes safety checks: If sleep_time_us is too short or negative, 
 * it defaults to a safe minimum duration to prevent boot loops.
 * * @param sleep_time_us Desired sleep time in microseconds.
 */
void goToDeepSleep(int64_t sleep_time_us);
