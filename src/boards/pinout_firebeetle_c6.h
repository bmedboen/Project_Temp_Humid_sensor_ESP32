#pragma once
#include <Arduino.h>

// --- DFRobot FireBeetle 2 ESP32-C6 Pinout ---

// I2C (Default for FireBeetle C6)
constexpr int I2C_SDA_PIN = 19;
constexpr int I2C_SCL_PIN = 20;

// DHT Sensor (Using GPIO 1 / D2)
constexpr uint8_t DHT_PIN = 1;

// Wakeup Button
// Note: On C6, we can use almost any GPIO. 
// GPIO 9 is often the BOOT button on C6 boards.
constexpr uint8_t BUTTON_PIN = 4; 

// OLED Reset (Standard)
constexpr int8_t OLED_RESET_PIN = -1;
