// dht_sensor.cpp

#include "dht_sensor.h"
#include "config.h"
#include "system_logger.h" 

#define LOG_TAG "DHT" 

// Initialize DHT object using the definitions from config.h/platformio.ini
// DHT_PIN comes from board definition
// DHT_TYPE_DEF comes from SENSOR_TYPE in platformio.ini
DHT dht(DHT_PIN, DHT_TYPE_DEF);

bool DHTSensor_init() {
  LOG_INFO(LOG_TAG, "Initializing DHT Sensor (Type ID: %d, Pin: %d)", DHT_TYPE_DEF, DHT_PIN);
  dht.begin();
  return true;
}

float DHTSensor_readHumidity() {
  return dht.readHumidity();
}

float DHTSensor_readTemperature() {
  return dht.readTemperature();
}
