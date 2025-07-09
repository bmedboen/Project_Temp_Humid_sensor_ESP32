// main.cpp

#include "DHT.h"       // For DHT object definition
#include <FS.h>        // For LittleFS.begin()
#include <LittleFS.h>  // For LittleFS.begin()
#include <time.h>      // For time_t, struct tm, mktime, localtime
#include <sys/time.h>  // For settimeofday
#include "esp_sleep.h" // For esp_sleep_enable_timer_wakeup, esp_sleep_get_wakeup_cause etc.

#include "config.h"
#include "web_server.h" // For Web Server functions

// --- Global DHT Sensor Object (using global dht for now) ---
DHT dht(DHT_PIN, DHT_TYPE);

// --- Forward Declarations for main.ino's own functions (TEMPORARY - will be moved) ---
String getFormattedTime();
void logSensorData();
void goToDeepSleep();

// --- Global function for formatted time (TEMPORARY - will be moved) ---
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Not Set";
  }
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}

// --- Data Logging Function (TEMPORARY - will be moved) ---
void logSensorData() {
  delay(100);

  float h = dht.readHumidity(); // Using global dht for now
  float t = dht.readTemperature();

  String dataString = "";
  String timestamp = getFormattedTime();

  if (isnan(h) || isnan(t)) {
    dataString = timestamp + ",Sensor Error,Sensor Error";
    Serial.println("Failed to read from DHT sensor for logging.");
  } else {
    dataString = timestamp + "," + String(h, 1) + "," + String(t, 1);
    Serial.print("Logged: ");
    Serial.println(dataString);
  }

  if (!LittleFS.begin()) {
    Serial.println("LittleFS not mounted, cannot log data.");
    return;
  }

  File dataFile = LittleFS.open(LOG_FILE_NAME, "a");
  if (dataFile) {
    if (dataFile.size() == 0) {
        dataFile.println("Timestamp,Humidity (%),Temperature (C)");
    }
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data saved to LittleFS.");
  } else {
    Serial.println("Error opening " + String(LOG_FILE_NAME) + " on LittleFS!");
  }
}

// --- Function to put ESP32 into deep sleep (TEMPORARY - will be moved) ---
void goToDeepSleep() {
  Serial.println("Entering Deep Sleep for " + String(LOG_INTERVAL_SECONDS) + " seconds (or until button press on GPIO " + String(BUTTON_PIN) + ")...");

  esp_sleep_enable_timer_wakeup(LOG_INTERVAL_SECONDS * 1000000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);

  esp_deep_sleep_start();
  Serial.println("Deep sleep failed! This should not happen.");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nBooting ESP32 Historical Data Logger...");

  dht.begin();
  delay(2000);
  Serial.println("DHT sensor initialized.");

  Serial.print("Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed. Attempting to format...");
    if (LittleFS.format()) {
      Serial.println("LittleFS formatted successfully!");
      if (!LittleFS.begin()) {
        Serial.println("LittleFS re-mount failed after format!");
      } else {
        Serial.println("LittleFS re-mounted after format.");
      }
    } else {
      Serial.println("LittleFS format failed!");
    }
  } else {
    Serial.println("LittleFS mounted successfully.");
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
      logSensorData();
      if (ENABLE_WEB_SERVER_ON_TIMER_WAKEUP) {
        activateWebServer = true;
      }
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup by touchpad (Not configured)"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup by ULP program (Not configured)"); break;
    default: // First boot or Reset
      Serial.println("Wakeup not caused by deep sleep (First boot or Reset)");
      logSensorData();
      activateWebServer = true;
      break;
  }

  if (activateWebServer) {
    initWebServerAP(); // Call modularized function
    setupWebServerRoutes(); // Call modularized function
    startWebServer(); // Call modularized function
    resetWebServerActivityTimer(); // Call modularized function
  } else {
    Serial.println("Web server NOT activated based on configuration.");
    Serial.println("Entering Deep Sleep immediately...");
    goToDeepSleep();
  }
}

void loop() {
  if (isWebServerActive()) { // Call modularized function
    handleWebServerClients(); // Call modularized function
    if (isWebServerTimeoutReached()) { // Call modularized function
      goToDeepSleep();
    }
  } else {
    goToDeepSleep();
  }

  delay(10);
}
