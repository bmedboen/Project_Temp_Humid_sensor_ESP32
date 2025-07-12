// data_logger.cpp

#include "data_logger.h"

#include "config.h"    // For LOG_FILE_NAME
#include "time_manager.h" // For getFormattedTime()

#include <cmath>       // For isnan()
#include <FS.h>        
#include <LittleFS.h>
#include "esp_system.h" // Needed for RTC_DATA_ATTR  

// Static global variables to store the last successfully logged values
// These need to be RTC_DATA_ATTR to persist across deep sleep
RTC_DATA_ATTR static float s_lastLoggedHumidity = NAN;
RTC_DATA_ATTR static float s_lastLoggedTemperature = NAN;

bool DataLogger_init() {
  Serial.print("DataLogger: Initializing LittleFS... ");
  if (!LittleFS.begin()) {
    Serial.println("Mount Failed. Attempting to format...");
    if (LittleFS.format()) {
      Serial.println("LittleFS formatted successfully!");
      if (!LittleFS.begin()) {
        Serial.println("LittleFS re-mount failed after format!");
        return false; // Return false if re-mount fails
      } else {
        Serial.println("LittleFS re-mounted after format.");
      }
    } else {
      Serial.println("LittleFS format failed!");
      return false; // Return false if format fails
    }
  } else {
    Serial.println("LittleFS mounted successfully.");
  }
  return true; // Return true on successful initialization
}

bool DataLogger_logSensorData(float temperature, float humidity) {

  String dataString = "";
  String timestamp = getFormattedTime(); 

  if (isnan(temperature) || isnan(humidity)) {
    dataString = timestamp + ",Sensor Error,Sensor Error";
    Serial.println("DataLogger: Failed to read from DHT sensor for logging.");
    // Return false on sensor read error
    return false;
  } else {
    dataString = timestamp + "," + String(humidity, 1) + "," + String(temperature, 1);
    Serial.print("DataLogger: Logged: ");
    Serial.println(dataString);

    // Update the static global variables with the successfully logged data
    s_lastLoggedTemperature = temperature;
    s_lastLoggedHumidity = humidity;    
  }

  File dataFile = LittleFS.open(LOG_FILE_NAME, "a"); // Use LOG_FILE_NAME from config.h
  if (dataFile) {
    if (dataFile.size() == 0) {
      // Write header if file is new or empty
      dataFile.println("Timestamp,Humidity (%),Temperature (C)");
    }
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("DataLogger: Data saved to LittleFS.");
    return true; // Return true on successful logging
  } else {
    Serial.println("DataLogger: Error opening " + String(LOG_FILE_NAME) + " on LittleFS!");
    return false; // Return false on file open error
  }
}

float DataLogger_getLastHumidity() {
    return s_lastLoggedHumidity;
}

float DataLogger_getLastTemperature() {
    return s_lastLoggedTemperature;
}
