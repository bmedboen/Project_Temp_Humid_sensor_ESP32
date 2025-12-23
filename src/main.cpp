// main.cpp

#include <Arduino.h>
#include "app_controller.h"
#include "config.h" 
#include "sleep_manager.h" // Needed for the actual sleep call
#include "system_logger.h" // New include for logging

#define LOG_TAG "MAIN" // Define a tag for Main module logs

// --- RTC Memory ---
// These must define storage here
extern "C" uint64_t esp_rtc_get_time_us();  // Function to get the current time in microseconds since boot. NB! Not public API.
RTC_DATA_ATTR uint64_t time_last_logged_ms = 0; 
RTC_DATA_ATTR int deep_sleep_count = 0;

AppState currentState = STATE_INIT;

// Runtime flags passed between states
bool stayAwakeForInteraction = false; 

void setup() {
    currentState = STATE_INIT;
}

void loop() {
    switch (currentState) {
        case STATE_INIT:
            currentState = run_state_init();
            break;

        case STATE_CHECK_WAKEUP:
            currentState = run_state_check_wakeup(stayAwakeForInteraction);
            break;

        case STATE_LOGGING:
             currentState = run_state_logging(stayAwakeForInteraction);
             break;

        case STATE_INTERACTIVE:
            currentState = run_state_interactive();
            break;

        case STATE_PREPARE_SLEEP:
            currentState = run_state_prepare_sleep();
            break;
    }
}
