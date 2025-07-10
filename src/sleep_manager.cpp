// sleep_manager.cpp

#include "sleep_manager.h"
#include "config.h"    // For LOG_INTERVAL_SECONDS, BUTTON_PIN

void goToDeepSleep() {
  Serial.println("Entering Deep Sleep for " + String(LOG_INTERVAL_SECONDS) + " seconds (or until button press on GPIO " + String(BUTTON_PIN) + ")...");

  esp_sleep_enable_timer_wakeup(LOG_INTERVAL_SECONDS * 1000000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);

  esp_deep_sleep_start();
  Serial.println("Deep sleep failed! This should not happen.");
}
