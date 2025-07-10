// main.cpp

#include <Arduino.h>   // Always needed for Arduino functions like Serial, delay, etc.
#include <time.h>      // For time_t, struct tm, mktime, localtime
#include <sys/time.h>  // For settimeofday
#include "esp_sleep.h" // For esp_sleep_enable_timer_wakeup, esp_sleep_get_wakeup_cause etc.

#include "config.h"
#include "data_logger.h" 
#include "dht_sensor.h"
#include "sleep_manager.h"
#include "time_manager.h" 
#include "web_server.h" 

void setup() {
  Serial.begin(115200);
  Serial.println("\nBooting ESP32 Historical Data Logger...");

  DHTSensor_init(); // Initialize DHT sensor
  delay(2000);
  Serial.println("DHT sensor initialized.");

  if (!DataLogger_init()) {
    Serial.println("Filesystem failed. Logging and web server will not work.");
  }
    
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  bool activateWebServer = false;

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup by EXT0 pin (Button Press on GPIO " + String(BUTTON_PIN) + ")");
      activateWebServer = true;
      break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup by EXT1 pin (Not configured)"); break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup by timer (" + String(LOG_INTERVAL_SECONDS) + " seconds)");
      DataLogger_logSensorData();
      if (ENABLE_WEB_SERVER_ON_TIMER_WAKEUP) {
        activateWebServer = true;
      }
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup by touchpad (Not configured)"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup by ULP program (Not configured)"); break;
    default: // First boot or Reset
      Serial.println("Wakeup not caused by deep sleep (First boot or Reset)");
      DataLogger_logSensorData();
      activateWebServer = true;
      break;
  }

  if (activateWebServer) {
    initWebServerAP(); 
    setupWebServerRoutes(); 
    startWebServer(); 
    resetWebServerActivityTimer(); 
  } else {
    Serial.println("Web server NOT activated based on configuration.");
    Serial.println("Entering Deep Sleep immediately...");
    goToDeepSleep();
  }
}

void loop() {
  if (isWebServerActive()) { 
    handleWebServerClients(); 
    if (isWebServerTimeoutReached()) { 
      goToDeepSleep();
    }
  } else {
    goToDeepSleep();
  }

  delay(10);
}
