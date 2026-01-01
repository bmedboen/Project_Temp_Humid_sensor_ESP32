// data_logger.cpp

#include "data_logger.h"
#include "system_logger.h" // New include for logging
#include "config.h"    // For LOG_FILE_NAME
#include "time_manager.h" // For getFormattedTime()

#include <cmath>       // For isnan()
#include <FS.h>        
#include <LittleFS.h>
#include "esp_system.h" // Needed for RTC_DATA_ATTR  

#define LOG_TAG "DATALOG" // Define a tag for DataLogger module logs

// Static global variables to store the last successfully logged values
// These need to be RTC_DATA_ATTR to persist across deep sleep
RTC_DATA_ATTR static float s_lastLoggedHumidity = NAN;
RTC_DATA_ATTR static float s_lastLoggedTemperature = NAN;
RTC_DATA_ATTR static char s_lastLoggedTime[32] = "N/A";

bool DataLogger_init() {
  LOG_DEBUG(LOG_TAG, "Initializing LittleFS...");

  if (!LittleFS.begin()) {
    LOG_WARN(LOG_TAG, "LittleFS mount failed. Attempting to format...");
    if (LittleFS.format()) {
      LOG_INFO(LOG_TAG, "LittleFS formatted successfully!");
      if (!LittleFS.begin()) {
        LOG_ERROR(LOG_TAG, "LittleFS re-mount failed after format!");
        return false; // Return false if re-mount fails
      } else {
        LOG_INFO(LOG_TAG, "LittleFS re-mounted after format.");
      }
    } else {
      LOG_ERROR(LOG_TAG, "LittleFS format failed!");
      return false; // Return false if format fails
    }
  } else {
    LOG_DEBUG(LOG_TAG, "LittleFS mounted successfully.");
  }
  return true; // Return true on successful initialization
}

bool DataLogger_logSensorData(float temperature, float humidity) {

  String dataString = "";
  String timestamp = getFormattedTime(); 

  if (isnan(temperature) || isnan(humidity)) {
    dataString = timestamp + ",Sensor Error,Sensor Error";
    LOG_ERROR(LOG_TAG, "Failed to read from DHT sensor for logging.");
    // Return false on sensor read error
    return false;
  } else {
    dataString = timestamp + "," + String(humidity, 1) + "," + String(temperature, 1);
    LOG_INFO(LOG_TAG, "Logged: %s", dataString.c_str());

    // Update the static global variables with the successfully logged data
    s_lastLoggedTemperature = temperature;
    s_lastLoggedHumidity = humidity;    

    strncpy(s_lastLoggedTime, timestamp.c_str(), sizeof(s_lastLoggedTime)); // Store timestamp
    s_lastLoggedTime[sizeof(s_lastLoggedTime) - 1] = 0; // Null-terminate for safety
  }

  File dataFile = LittleFS.open(LOG_FILE_NAME, "a"); // Use LOG_FILE_NAME from config.h
  if (dataFile) {
    if (dataFile.size() == 0) {
      // Write header if file is new or empty
      dataFile.println("Timestamp,Humidity (%),Temperature (C)");
    }
    dataFile.println(dataString);
    dataFile.close();

    LOG_INFO(LOG_TAG, "Saved entry: %s", dataString.c_str());
    return true; // Return true on successful logging

  } else {
    LOG_ERROR(LOG_TAG, "Error opening %s on LittleFS!", LOG_FILE_NAME);
    return false; // Return false on file open error
  }
}

float DataLogger_getLastHumidity() {
    return s_lastLoggedHumidity;
}

float DataLogger_getLastTemperature() {
    return s_lastLoggedTemperature;
}

String DataLogger_getLastLogTime() {
    return String(s_lastLoggedTime);
}
