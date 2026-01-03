#pragma once
#include <Arduino.h>

// --- Generic ESP32 DevKit V1 Pinout ---

// I2C (Standard ESP32)
constexpr int I2C_SDA_PIN = 21;
constexpr int I2C_SCL_PIN = 22;

// DHT Sensor
constexpr uint8_t DHT_PIN = 18;

// Wakeup Button
constexpr uint8_t BUTTON_PIN = 4;

// OLED Reset
constexpr int8_t OLED_RESET_PIN = -1;
