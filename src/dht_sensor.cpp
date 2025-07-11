// dht_sensor.cpp

#include "config.h"

// Create DHT object
DHT dht(DHT_PIN, DHT_TYPE);

bool DHTSensor_init() {
  dht.begin();
  // Serial.println("DHT Sensor initialized.");
  // A brief read to ensure it's working
  // float h = dht.readHumidity();
  // float t = dht.readTemperature();
  // if (isnan(h) || isnan(t)) {
  //   Serial.println("DHT Sensor: Failed to read initial sensor data!");
  //   return false;
  // }
  return true;
}

float DHTSensor_readHumidity() {
  return dht.readHumidity();
}

float DHTSensor_readTemperature() {
  return dht.readTemperature();
}
