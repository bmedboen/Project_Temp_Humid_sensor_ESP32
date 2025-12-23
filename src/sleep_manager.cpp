// sleep_manager.cpp

#include "sleep_manager.h"
#include "config.h"
#include "esp_sleep.h"

// Define safety constants locally
static const int64_t MIN_SLEEP_US = 1000000;      // 1 second minimum allowed sleep
static const int64_t SAFE_DEFAULT_US = 2000000;  // 2 seconds default if calculation fails

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
        Serial.print("Sleep Manager: Warning! Requested sleep time (");
        Serial.print((long)(sleep_time_us / 1000)); 
        Serial.println(" ms) is too short or negative.");
        
        sleep_time_us = SAFE_DEFAULT_US;
        
        Serial.println("Sleep Manager: Enforcing safe default of 10 seconds.");
    }

    Serial.println("Sleep Manager: Entering Deep Sleep for " + String((long)(sleep_time_us / 1000000)) + " seconds...");
    Serial.println("Sleep Manager: Wake up source: Timer or Button (GPIO " + String(BUTTON_PIN) + ")");

    // Configure Timer Wakeup (Casting back to uint64_t is safe here because we ensured it's positive above)
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_time_us);

    // Configure Button Wakeup
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);

    Serial.flush(); // Ensure all serial data is sent before shutting down the radio/CPU
    esp_deep_sleep_start();
  
    // This line will never be reached unless deep sleep fails
    Serial.println("Sleep Manager: Error! Deep sleep failed.");
}
