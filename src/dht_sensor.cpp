// dht_sensor.cpp

#include "config.h"

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
