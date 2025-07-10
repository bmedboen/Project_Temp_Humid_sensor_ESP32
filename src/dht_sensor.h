// dht_sensor.h

#pragma once

#include <DHT.h>

/**
 * @brief Initializes the DHT sensor.
 * @return true if initialized successfully, false otherwise.
 */
bool DHTSensor_init();

/**
 * @brief Reads the humidity from the DHT sensor.
 * @return Humidity value as float, or NaN on error.
 */
float DHTSensor_readHumidity();

/**
 * @brief Reads the temperature from the DHT sensor.
 * @return Temperature value as float, or NaN on error.
 */
float DHTSensor_readTemperature();
