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

void setup() {
  delay(500); // Stabilize after power on/reset
  Serial.begin(115200);

  // Serial.setTxTimeoutMs(0); 

  // 2. Vent på PC-tilkobling (5 sekunder er bra)
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000)) {
      delay(10);
  }

  // 3. KRITISK: Gi USB-bufferen på PC-en tid til å stabilisere seg
  // Etter at !Serial blir true, trenger OS-en ofte noen millisekunder 
  // på å faktisk begynne å tegne i terminalvinduet.
  delay(1000); 

  // 4. Send en tydelig "banner" for å tømme PC-bufferen
  Serial.println("\n\n\n");
  Serial.println("========================================");
  Serial.println("       ESP32-C6 BOOT SEQUENCER          ");
  Serial.println("========================================");
  
  // 5. Nå kan du logge de første meldingene
  LOG_INFO(LOG_TAG, "--- System Starting Up ---");
  
  // Nå vil resten av loopen og tilstandsmaskinen starte
  
  // Gi PC-en et lite halvsekund på å faktisk tegne teksten over
  delay(500);
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
