// config.h

#pragma once

#include <cstdint>
#include <DHT.h>

// --- Wi-Fi Access Point (AP) Configuration ---
// Make sure these are const char* for string literals.
// Remember to change to something secure for ap_password!
constexpr const char* AP_SSID = "ESP32_DataLogger";
constexpr const char* AP_PASSWORD = "12345678"; // <<< CHANGE THIS TO SOMETHING SECURE! >>>

// --- DHT Sensor Configuration ---
constexpr uint8_t DHT_PIN = 18;
constexpr uint8_t DHT_TYPE = DHT11; // Assuming DHT11. Adjust if you use DHT22, etc.

// --- LittleFS Configuration ---
constexpr const char* LOG_FILE_NAME = "/datalog.csv";

// --- Logging and Sensor Read Intervals ---
// Use unsigned long for time intervals to match millis() and avoid issues
constexpr unsigned long LOG_INTERVAL_SECONDS = 60;          // How often to log data (e.g., 60 seconds = 1 minute)
constexpr unsigned long DHT_READ_INTERVAL_MS = 2000;        // How often to read DHT sensor (e.g., 2000ms = 2 seconds)

// --- Button Configuration for Wake-up ---
constexpr uint8_t BUTTON_PIN = 4; // Using GPIO 4 for EXT0 wake-up

// --- Web Server Configuration ---
constexpr unsigned int WEBSERVER_PORT = 80; // Standard HTTP port
constexpr unsigned long WEB_SERVER_INACTIVITY_TIMEOUT = 60 * 1000; // 60 seconds (1 minute) inactivity to go to sleep

// NEW CONFIGURATION VARIABLE (Set to true if you want Web Server on Timer Wakeup)
constexpr bool ENABLE_WEB_SERVER_ON_TIMER_WAKEUP = false; // <<< --- ADJUST THIS VALUE --- >>>
