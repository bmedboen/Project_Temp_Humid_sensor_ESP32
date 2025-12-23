// sleep_manager.cpp

#include "sleep_manager.h"
#include "config.h"
#include "esp_sleep.h"
#include "system_logger.h" // New include for logging

#define LOG_TAG "SLEEP" // Define a tag for Sleep Manager module logs

int64_t calculateSleepTime(uint64_t timeLastLogged_ms, uint64_t currentTime_ms, uint64_t overhead_ms, uint64_t logInterval_ms) {
    // Explicitly cast all unsigned inputs to signed 64-bit integers BEFORE calculation.
    // This prevents unsigned integer underflow (wrapping around to huge numbers)
    // if the result becomes negative (e.g., if we spent too much time awake).
    
    int64_t interval = (int64_t)logInterval_ms;
    int64_t overhead = (int64_t)overhead_ms;
    int64_t current  = (int64_t)currentTime_ms;
    int64_t last     = (int64_t)timeLastLogged_ms;

    // Calculate time elapsed since last log
    int64_t time_since_last_log = current - last;

    // Calculate remaining sleep time in milliseconds
    int64_t remaining_time_ms = interval - overhead - time_since_last_log;

    // Convert to microseconds
    return remaining_time_ms * 1000LL;
}

void goToDeepSleep(int64_t sleep_time_us) {
  
    // --- SAFETY CHECK ---
    // Ensure we don't try to sleep for a negative time or an extremely short time.
    // Short deep sleep cycles can cause boot loops that are hard to recover from.
    if (sleep_time_us < MIN_SLEEP_US) {
        LOG_WARN(LOG_TAG, "Requested sleep time (%.3f s) is too short or negative.", (double)sleep_time_us / 1000000.0);
        
        sleep_time_us = SAFE_DEFAULT_US;
        
        LOG_INFO(LOG_TAG, "Enforcing safe default of %.3f seconds.", (double)SAFE_DEFAULT_US / 1000000.0);
    }

    LOG_INFO(LOG_TAG, "Entering Deep Sleep for %.3f seconds...", (double)sleep_time_us / 1000000.0);
    LOG_INFO(LOG_TAG, "Wake up source: Timer or Button (GPIO %d)", BUTTON_PIN);

    // Configure Timer Wakeup (Casting back to uint64_t is safe here because we ensured it's positive above)
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_time_us);

    // Configure Button Wakeup
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);

    Serial.flush(); // Ensure all serial data is sent before shutting down the radio/CPU
    esp_deep_sleep_start();
  
    // This line will never be reached unless deep sleep fails
    LOG_ERROR(LOG_TAG, "Deep sleep failed! This should not happen.");
}
