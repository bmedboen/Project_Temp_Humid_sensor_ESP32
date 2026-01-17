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

    // 2. Configure Button Wakeup (Hardware Specific)
    // We configure the pin as INPUT_PULLUP first
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    #if defined(BOARD_FIREBEETLE_C6)
        // --- ESP32-C6 / RISC-V Implementation (IDF 5.x / Arduino 3.x) ---
        // On the C6, ext0_wakeup is deprecated/removed. We use gpio_wakeup.
        // We must enable the GPIO wake-up logic for the specific pin, requiring LOW level.


        
        LOG_DEBUG(LOG_TAG, "Configuring Wakeup for RISC-V (C6)");

        // Ensure the pin direction and pull-up state is maintained during sleep
        gpio_set_direction((gpio_num_t)BUTTON_PIN, GPIO_MODE_INPUT);
        gpio_pullup_en((gpio_num_t)BUTTON_PIN);

        // Enable deep sleep wakeup for the specific pin bitmask
        // We use 1ULL << BUTTON_PIN to target the correct GPIO
        esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
        

        // // Enable wakeup for the pin (Low level triggers wakeup)
        // gpio_wakeup_enable((gpio_num_t)BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
        
        // // Enable GPIO wakeup source globally for sleep
        // esp_sleep_enable_gpio_wakeup();

    #else
        // --- ESP32 Classic / Xtensa Implementation ---
        // Uses the traditional EXT0 wakeup source for RTC IOs.
        
        LOG_DEBUG(LOG_TAG, "Configuring Wakeup for Classic ESP32");
        esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
        
    #endif

    Serial.flush(); // Ensure all serial data is sent before shutting down the radio/CPU
    delay(100); // Give the USB stack time to push the final buffer to the PC
    
    esp_deep_sleep_start();
  
    // This line will never be reached unless deep sleep fails
    LOG_ERROR(LOG_TAG, "Deep sleep failed! This should not happen.");
}
