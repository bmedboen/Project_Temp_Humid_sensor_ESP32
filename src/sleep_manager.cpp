// sleep_manager.cpp

#include "sleep_manager.h"
#include "config.h"

void goToDeepSleep(unsigned long deep_sleep_time_us) {
  Serial.println("Sleep_manager: Entering Deep Sleep for " + String(deep_sleep_time_us / 1000000) + " seconds (or until button press on GPIO " + String(BUTTON_PIN) + ")...");

  esp_sleep_enable_timer_wakeup(deep_sleep_time_us);  //Time in microseconds

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);

  esp_deep_sleep_start();
  Serial.println("Sleep: manager: Deep sleep failed! This should not happen.");
}
