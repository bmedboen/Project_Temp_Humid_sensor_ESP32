// oled_display.h

#pragma once

#include <Arduino.h>

/**
 * @brief Initializes the OLED display using Adafruit_SSD1306 library.
 * @return true if initialized successfully, false otherwise.
 */
bool OLEDDisplay_init();

/**
 * @brief Clears the OLED display.
 */
void OLEDDisplay_clear();

/**
 * @brief Displays temperature and humidity on the OLED screen.
 * @param temperature The temperature value to display.
 * @param humidity The humidity value to display.
 */
void OLEDDisplay_showSensorData(float temperature, float humidity);

/**
 * @brief Displays connection status, IP address and Hostname (URL).
 */
void OLEDDisplay_showNetworkStatus(String ip, String hostname);

/**
 * @brief Turns off the OLED display to save power.
 */
void OLEDDisplay_turnOff();
