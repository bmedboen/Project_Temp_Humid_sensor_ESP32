// app_controller.h
#pragma once

#include <Arduino.h>

// Define the possible states of the application
enum AppState {
    STATE_INIT,
    STATE_CHECK_WAKEUP,
    STATE_LOGGING,
    STATE_INTERACTIVE,
    STATE_PREPARE_SLEEP
};

// --- State Handler Functions ---
// Each function runs the logic for that state and returns the NEXT state to transition to.

AppState run_state_init();
AppState run_state_check_wakeup(bool &stayAwakeFlag); // We pass the flag by reference to change it
AppState run_state_logging(bool stayAwakeFlag);
AppState run_state_interactive();
AppState run_state_prepare_sleep();  // We pass sleepTime variable to update it
