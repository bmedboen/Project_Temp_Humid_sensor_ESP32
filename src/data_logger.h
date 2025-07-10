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
bool DataLogger_logSensorData();
