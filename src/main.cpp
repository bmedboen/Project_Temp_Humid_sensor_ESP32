// main.cpp

#include <Arduino.h>
#include "app_controller.h"
#include "config.h" 
#include "sleep_manager.h" // Needed for the actual sleep call
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

// Ensure we only dump history once per boot/connection
bool hasDumpedHistory = false;

void setup() {
  // 1. Initialize Serial without blocking
  // We set timeout to 0 so the code doesn't freeze if USB is unplugged.
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); 

  // 2. Initialize RTC Logging
  Logger_Init();
  
  // 3. Log Startup (This goes instantly to RTC Memory)
  // We do NOT wait for PC here. Fast boot!
  LOG_INFO(LOG_TAG, "--- System Starting Up (Fast Boot) ---");
  LOG_INFO(LOG_TAG, "Wakeup Cause: %d", (int)esp_sleep_get_wakeup_cause());

  // 4. SYNCHRONIZATION POINT (The "Wait for Developer" Logic)
  // We try to detect the PC for up to 4 seconds.
  // This gives you time to open the Serial Monitor after a sleep cycle.
  
  unsigned long waitStart = millis();
  bool pcConnected = false;

  while ((millis() - waitStart < 4000)) {
      if (Serial) {
          pcConnected = true;
          // We found the PC! Break the loop early.
          break; 
      }
      delay(10);
  }

  // 5. DUMP HISTORY
  if (pcConnected) {
      // Give the PC driver a tiny moment to stabilize the text stream
      delay(500); 
      
      // Dump the logs we captured in Step 3
      Logger_FlushRTCtoSerial();
      
      LOG_INFO(LOG_TAG, "PC Connection established. Starting application...");
  } else {
      // Timeout reached (Battery mode)
      LOG_INFO(LOG_TAG, "No PC detected (Timeout). Starting application in headless mode.");
  }
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
