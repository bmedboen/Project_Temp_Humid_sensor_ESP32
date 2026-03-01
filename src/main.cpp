// main.cpp

#include <Arduino.h>
#include "app_controller.h"
#include "config.h" 
#include "sleep_manager.h"
#include "system_logger.h" 

#define LOG_TAG "MAIN"

// --- Global Variables ---
// RTC variables to persist across deep sleep cycles
extern "C" uint64_t esp_rtc_get_time_us();  // Function to get the current time in microseconds since boot. NB! Not public API.
RTC_DATA_ATTR uint64_t time_last_logged_ms = 0; 
RTC_DATA_ATTR int deep_sleep_count = 0;

// State Machine Variables 
static AppState currentState = STATE_INIT;
static AppState lastLoggedState = STATE_NONE;

// Runtime flags passed between states
bool stayAwakeForInteraction = false; 

void setup() {
  // 1. Initialize Serial without blocking
  // We set timeout to 0 so the code doesn't freeze if USB is unplugged.
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  
  // 2. DEVELOPMENT WAIT LOOP (The "Magic Pause")
  // In Production (CORE_DEBUG_LEVEL = 0), this code is removed entirely.
  #if CORE_DEBUG_LEVEL > 0
      unsigned long waitStart = millis();
      // Wait up to 3 seconds for the Serial Monitor to open
      while (!Serial && (millis() - waitStart < 3000)) {
          delay(10);
      }
      
      if (Serial) {
          Serial.println("\n\n--- [DEV MODE] SYSTEM AWAKE & TERMINAL READY ---");
      }
  #endif

  // 3. Initialize File System Logging
  Logger_Init();
  
  // 4. Log Startup
  LOG_INFO(LOG_TAG, "--- System Starting Up (Fast Boot) ---");
  LOG_INFO(LOG_TAG, "Wakeup Cause: %d", (int)esp_sleep_get_wakeup_cause());
}


void loop() {

  // Check for state transition (for logging/debugging)
  if (currentState != lastLoggedState) {
        
        LOG_INFO(LOG_TAG, ">> STATE TRANSITION: %s -> %s", 
                 getAppStateName(lastLoggedState), 
                 getAppStateName(currentState));
        
        // Update the history
        lastLoggedState = currentState;
    }
  
  // 3. Run the State Machine
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
