// config.h

#pragma once

#include <cstdint>
#include <DHT.h>

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
constexpr unsigned long LOG_INTERVAL_SECONDS = 60; // How often to log data in seconds
constexpr unsigned long WAKEUP_OVERHEAD_MS = 1000; // Estimated time for boot + setup + pre-log ops in milliseconds
constexpr unsigned long DEEP_SLEEP_TIMER_US = (LOG_INTERVAL_SECONDS * 1000000UL) - (WAKEUP_OVERHEAD_MS * 1000UL); // In microseconds

// --- Button Configuration for Wake-up ---
constexpr uint8_t BUTTON_PIN = 4; // Using GPIO 4 for EXT0 wake-up

// --- Web Server Configuration ---
constexpr unsigned int WEBSERVER_PORT = 80; // Standard HTTP port
constexpr unsigned long WEB_SERVER_INACTIVITY_TIMEOUT = 90 * 1000; // Inactivity in seconds to go to sleep
constexpr bool ENABLE_WEB_SERVER_ON_TIMER_WAKEUP = false; // Activate Web Server on Timer Wakeup

// --- OLED Display Configuration ---
constexpr uint8_t OLED_RESET_PIN = -1; // Reset pin for SSD1306 (or -1 if sharing Arduino reset)
constexpr uint8_t OLED_SCREEN_WIDTH = 128; // OLED display width, in pixels
constexpr uint8_t OLED_SCREEN_HEIGHT = 64; // OLED display height, in pixels
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C; // OLED I2C address (can be 0x3C or 0x3D)
constexpr unsigned long OLED_DISPLAY_DURATION_MS = 5000; // How long the OLED display stays on after button press (in milliseconds)
