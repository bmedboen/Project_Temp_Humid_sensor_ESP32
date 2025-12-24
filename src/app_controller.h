// app_controller.h
#pragma once

#include <Arduino.h>

// Define the possible states of the application
enum AppState {
    STATE_NONE,
    STATE_INIT,
    STATE_CHECK_WAKEUP,
    STATE_LOGGING,
    STATE_INTERACTIVE,
    STATE_PREPARE_SLEEP
};

// Utility function to get the name of the state as a string (for logging/debugging)
inline const char* getAppStateName(AppState state) {
    switch (state) {
        case STATE_NONE:          return "NONE";
        case STATE_INIT:          return "INIT";
        case STATE_CHECK_WAKEUP:  return "CHECK_WAKEUP";
        case STATE_LOGGING:       return "LOGGING";
        case STATE_INTERACTIVE:   return "INTERACTIVE";
        case STATE_PREPARE_SLEEP: return "PREPARE_SLEEP";
        default:                  return "UNKNOWN";
    }
}

// --- State Handler Functions ---
// Each function runs the logic for that state and returns the NEXT state to transition to.
AppState run_state_init();
AppState run_state_check_wakeup(bool &stayAwakeFlag); // We pass the flag by reference to change it
AppState run_state_logging(bool stayAwakeFlag);
AppState run_state_interactive();
AppState run_state_prepare_sleep();
