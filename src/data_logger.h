// data_logger.h

#pragma once

#include <Arduino.h>   // For String type

// --- External Dependencies ---
extern String getFormattedTime(); // Assuming getFormattedTime() is globally accessible (e.g., from time_manager.h)

// --- Public Functions ---
/**
 * @brief Initializes the LittleFS file system for data logging.
 * Formats LittleFS if mounting fails.
 * @return true if LittleFS is successfully mounted, false otherwise.
 */
bool DataLogger_init();

/**
 * @brief Logs sensor data (humidity and temperature) to the configured log file.
 * Writes header if the file is empty.
 * @return true if data was successfully logged, false on error.
 */
bool DataLogger_logSensorData(float temperature, float humidity);

/**
 * @brief Gets the last successfully logged humidity value.
 * @return The last humidity value, or NaN if no data has been logged yet or on error.
 */
float DataLogger_getLastHumidity();

/**
 * @brief Gets the last successfully logged temperature value.
 * @return The last temperature value, or NaN if no data has been logged yet or on error.
 */
float DataLogger_getLastTemperature();

/**
 * @brief Gets the timestamp of the LAST successful logging event.
 * Retreived from RTC memory.
 * @return String containing the time (e.g. "2025-01-01 12:00:00").
 */
String DataLogger_getLastLogTime(); 
