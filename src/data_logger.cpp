// data_logger.cpp

#include "data_logger.h"

#include "config.h"    // For LOG_FILE_NAME
#include "dht_sensor.h" 
#include "time_manager.h" // For getFormattedTime()

#include <cmath>       // For isnan()
#include <FS.h>        // Now included here, as File type is used in implementation
#include <LittleFS.h>  // Now included here, as LittleFS functions are called in implementation

bool DataLogger_logSensorData() {
  delay(100); // Small delay before reading DHT

  float h = DHTSensor_readHumidity();
  float t = DHTSensor_readTemperature();

  String dataString = "";
  String timestamp = getFormattedTime(); 

  if (isnan(h) || isnan(t)) {
    dataString = timestamp + ",Sensor Error,Sensor Error";
    Serial.println("DataLogger: Failed to read from DHT sensor for logging.");
    // Return false on sensor read error
    return false;
  } else {
    dataString = timestamp + "," + String(h, 1) + "," + String(t, 1);
    Serial.print("DataLogger: Logged: ");
    Serial.println(dataString);
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
