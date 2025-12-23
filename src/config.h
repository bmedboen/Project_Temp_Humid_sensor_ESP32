// config.h

#pragma once

#include <cstdint>
#include <Arduino.h>
#include <DHT.h>

// --- SYSTEM LOGGING CONFIGURATION ---
// 0 = NONE  (Silent)
// 1 = ERROR (Only critical failures)
// 2 = WARN  (Warnings and Errors)
// 3 = INFO  (Standard operational events)
// 4 = DEBUG (Detailed output for troubleshooting)

// Change this value to control output:
#define SYSTEM_LOG_LEVEL  4

// --- Wi-Fi Access Point (AP) Configuration ---
// Make sure these are const char* for string literals.
constexpr const char* AP_SSID = "ESP32_DataLogger";
constexpr const char* AP_PASSWORD = "12345678"; 

// --- DHT Sensor Configuration ---
constexpr uint8_t DHT_PIN = 18;
constexpr uint8_t DHT_TYPE = DHT11; // Assuming DHT11. Adjust if you use DHT22, etc.

// --- LittleFS Configuration ---
constexpr const char* LOG_FILE_NAME = "/datalog.csv";

// --- Logging and Sensor Read Intervals ---
constexpr unsigned long LOG_INTERVAL_SECONDS = 60 * 1; // How often to log data in seconds
constexpr unsigned long WAKEUP_OVERHEAD_MS = 1000; // Estimated time for boot + setup + pre-log ops in milliseconds

// --- Button Configuration for Wake-up ---
constexpr uint8_t BUTTON_PIN = 4; // Using GPIO 4 for EXT0 wake-up

// --- Web Server Configuration ---
constexpr unsigned int WEBSERVER_PORT = 80; // Standard HTTP port
constexpr unsigned long WEB_SERVER_INACTIVITY_TIMEOUT = 90 * 1000; // Inactivity in ms to go to sleep
constexpr bool ENABLE_WEB_SERVER_ON_TIMER_WAKEUP = false; // Activate Web Server on Timer Wakeup

// --- OLED Display Configuration ---
constexpr uint8_t OLED_RESET_PIN = -1; // Reset pin for SSD1306 (or -1 if sharing Arduino reset)
constexpr uint8_t OLED_SCREEN_WIDTH = 128; // OLED display width, in pixels
constexpr uint8_t OLED_SCREEN_HEIGHT = 64; // OLED display height, in pixels
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C; // OLED I2C address (can be 0x3C or 0x3D)

// --- Sleep Manager Configuration ---
constexpr int64_t MIN_SLEEP_US = 1000000;      // 1 second minimum allowed sleep
constexpr int64_t SAFE_DEFAULT_US = 2000000;   // 2 seconds default if calculation fails or too short