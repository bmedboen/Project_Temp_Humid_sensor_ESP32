// src/config.h
#pragma once

#include <cstdint>
#include <Arduino.h>
#include <DHT.h>

// --------------------------------------------------------------------------
// 1. HARDWARE SELECTION (Pinout)
// --------------------------------------------------------------------------
// These flags are defined in platformio.ini

#if defined(BOARD_FIREBEETLE_C6)
    #include "boards/pinout_firebeetle_c6.h"
#elif defined(BOARD_ESP32_CLASSIC)
    #include "boards/pinout_esp32_classic.h"
#else
    #error "Hardware board not defined! Check platformio.ini build_flags."
#endif

// --------------------------------------------------------------------------
// 2. SENSOR SELECTION
// --------------------------------------------------------------------------
#ifndef SENSOR_TYPE
    #warning "No SENSOR_TYPE defined. Defaulting to DHT11."
    #define SENSOR_TYPE DHT11
#endif

// DHT_TYPE is used by the DHT library (DHT11, DHT22, etc.)
constexpr uint8_t DHT_TYPE_DEF = SENSOR_TYPE; 

// --------------------------------------------------------------------------
// 3. GENERAL SYSTEM CONFIGURATION
// --------------------------------------------------------------------------

// Logging Level
#define SYSTEM_LOG_LEVEL  4 

// Wi-Fi Configuration
constexpr const char* AP_SSID = "ESP32_DataLogger";
constexpr const char* AP_PASSWORD = "12345678"; 
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;

// NTP / Time
constexpr const char* NTP_SERVER = "pool.ntp.org";
constexpr long  GMT_OFFSET_SEC = 3600;      // UTC+1
constexpr int   DAYLIGHT_OFFSET_SEC = 3600; // DST +1h
constexpr int   NTP_TIMEOUT_MS = 10000;

// External RTC
constexpr bool HAS_EXTERNAL_RTC = true;

// File System
constexpr const char* LOG_FILE_NAME = "/datalog.csv";

// Logic Intervals
constexpr unsigned long LOG_INTERVAL_SECONDS = 5 * 60; 
constexpr unsigned long WAKEUP_OVERHEAD_MS = 1000;

// Web Server
constexpr unsigned int WEBSERVER_PORT = 80;
constexpr unsigned long WEB_SERVER_INACTIVITY_TIMEOUT = 90 * 1000;
constexpr bool ENABLE_WEB_SERVER_ON_TIMER_WAKEUP = false; 
constexpr const char* MDNS_HOSTNAME = "esp32logger";

// OLED Display (Address and Size)
constexpr uint8_t OLED_SCREEN_WIDTH = 128;
constexpr uint8_t OLED_SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

// Sleep Safety
constexpr int64_t MIN_SLEEP_US = 1000000;      
constexpr int64_t SAFE_DEFAULT_US = 2000000;
