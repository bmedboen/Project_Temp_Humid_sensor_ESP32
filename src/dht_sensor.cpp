// dht_sensor.cpp

#include "config.h"
#include "system_logger.h" // New include for logging

#define LOG_TAG "DHT" // Define a tag for DHT module logs

// Create DHT object
DHT dht(DHT_PIN, DHT_TYPE);

bool DHTSensor_init() {
  dht.begin();
  return true;
}

float DHTSensor_readHumidity() {
  return dht.readHumidity();
}

float DHTSensor_readTemperature() {
  return dht.readTemperature();
}
