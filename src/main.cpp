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
#include "oled_display.h"

extern "C" uint64_t esp_rtc_get_time_us(); // Function to get the current time in microseconds since boot

RTC_DATA_ATTR uint64_t time_last_logged_s = 0; // Last time data was logged. Not reset on deep sleep. NB! Not public API.
RTC_DATA_ATTR long deep_sleep_time_us; // Not reset on deep sleep
RTC_DATA_ATTR int deep_sleep_count_s = 0; // Not reset on deep sleep

bool data_logged = false; // Flag to indicate if data was logged in the current cycle

// Global variables for OLED display management
bool oled_active = false;
unsigned long oled_active_start_time = 0;

long calculateDeepSleepTime() {
  // Adjust deep sleep time based on how long the system has been awake since last logging
  long deep_sleep_time_us = DEEP_SLEEP_TIMER_US - (esp_rtc_get_time_us() - time_last_logged_s * 1000UL); // Time in microseconds;

  return deep_sleep_time_us;
}

void setup() {
  unsigned long startSetupTime = millis(); // Record time at the very beginning of setup()

  Serial.begin(115200);
  Serial.println("\nMain: Booting ESP32 Historical Data Logger...");
  Serial.println("\nMain: Current number of deep sleep cycles: " + String(deep_sleep_count_s));

  // Initialize DHT sensor
  if (!DHTSensor_init()) {
    Serial.println("Main: DHT sensor initialization failed. Web server and logging will not work.");
  } else {
    Serial.println("Main: DHT sensor initialized successfully.");
  }

  // Initialize Data Logger
  if (!DataLogger_init()) {
    Serial.println("Main: Filesystem failed. Logging and web server will not work.");
  }

  // Initialize OLED Display (quick initialization)
  if (!OLEDDisplay_init()) {
      Serial.println("Main: OLED display initialization failed.");
  } else {
      Serial.println("Main: OLED display initialized successfully.");
  }
    
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  bool activateWebServer = false;

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Main: Wakeup by EXT0 pin (Button Press)");
    activateWebServer = true;
    oled_active = true;

  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Main: Wakeup by timer (" + String(deep_sleep_time_us / 1000000) + " seconds)");
    activateWebServer = ENABLE_WEB_SERVER_ON_TIMER_WAKEUP;
  } else {
    Serial.println("Main: Wakeup not caused by deep sleep (First boot or Reset or other reason)");
    activateWebServer = true;
    oled_active = true;
  }

  if (activateWebServer) {
    initWebServerAP(); 
    setupWebServerRoutes(); 
    startWebServer(); 
    resetWebServerActivityTimer(); 
  } else {
    Serial.println("Main: Web server NOT activated based on configuration. Will proceed to log and then deep sleep.");
  }

  unsigned long endSetupTime = millis(); // Record time at the very end of setup()
  Serial.println("Main: setup() execution time: " + String(endSetupTime - startSetupTime) + " ms");
}

// Read and log sensor data every LOG_INTERVAL_SECONDS
void loop() {  
  uint64_t current_time = esp_rtc_get_time_us() / 1000; //Time in milliseconds since boot   

  if (current_time - time_last_logged_s >= LOG_INTERVAL_SECONDS * 1000UL || time_last_logged_s == 0) {
    float currentLoggedHumidity = DHTSensor_readHumidity();
    float currentLoggedTemperature = DHTSensor_readTemperature();

    time_last_logged_s = current_time; // Update last logged time

    if (DataLogger_logSensorData(currentLoggedTemperature, currentLoggedHumidity)) {      
      Serial.println("Main: Data logged successfully.");
    } else {
      Serial.println("Main: Failed to log data.");
    }

    data_logged = true;
  }

  if (oled_active) {
      // Get last successfully logged data for OLED display
    float lastHumidity = DataLogger_getLastHumidity();
    float lastTemperature = DataLogger_getLastTemperature();

    // Display the last known sensor data on OLED
    OLEDDisplay_showSensorData(lastTemperature, lastHumidity);
  }

  // If data is logged, go to deep sleep, unless web server is active and timeout not reached. 
  // Then calculate the additional awake time. This additional awake time is used to calculate the next deep sleep time, 
  // to ensure we don't miss the next logging interval. 
  // Check deep_sleep_time is bigger than WAKEUP_OVERHEAD_MS * 1000UL to avoid going to deep sleep too soon. 
  if (isWebServerActive()) { 
    handleWebServerClients(); 
    if (isWebServerTimeoutReached() && data_logged) {          

      deep_sleep_time_us = calculateDeepSleepTime() - WAKEUP_OVERHEAD_MS * 1000UL;
      if (deep_sleep_time_us >= WAKEUP_OVERHEAD_MS * 1000UL) {

        OLEDDisplay_clear();
        OLEDDisplay_turnOff(); // Turn off the OLED display
        oled_active = false;
        Serial.println("Main: OLED display turned off.");

        ++deep_sleep_count_s;
        Serial.println("Main: Cycle time awake is " + String(millis() / 1000) + " seconds.");
        goToDeepSleep(deep_sleep_time_us);
      }      
    }
  } else if (data_logged) {
    deep_sleep_time_us = calculateDeepSleepTime() - WAKEUP_OVERHEAD_MS * 1000UL;
    if (deep_sleep_time_us >= WAKEUP_OVERHEAD_MS * 1000UL) {
      ++deep_sleep_count_s;
      Serial.println("Main: Cycle time awake is " + String(millis() / 1000) + " seconds.");
      goToDeepSleep(deep_sleep_time_us);
    }     
  }

  delay(10);
}
