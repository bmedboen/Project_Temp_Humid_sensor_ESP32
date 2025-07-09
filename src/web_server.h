// web_server.h

#pragma once

#include <WebServer.h> // Include the WebServer library
#include <WiFi.h>      // Include WiFi for softAP functions
#include <Arduino.h>   // For millis(), String, etc.
#include <FS.h>        // For LittleFS operations used in web server handlers
#include <LittleFS.h>
#include "DHT.h"       // For DHT readings in web server handlers

#include "config.h"

// --- Global WebServer instance and activity tracking ---
extern WebServer server; // Declare the WebServer object as extern
extern unsigned long webServerLastActivityTime;

// --- Global variables/objects from main.ino that web_server needs ---
extern DHT dht; // Assuming DHT dht is still globally declared in main.ino

// --- Forward Declarations of functions that will be defined in web_server.cpp ---
void initWebServerAP();
void setupWebServerRoutes();
void startWebServer();
void handleWebServerClients();
bool isWebServerActive();
void resetWebServerActivityTimer();
bool isWebServerTimeoutReached(); // Checks if inactivity timeout has been reached

// --- Forward Declarations of functions used by web_server but defined elsewhere ---
// These will also be modularized later
extern String getFormattedTime(); // From time_utils.h/cpp
extern void logSensorData();     // From data_logger.h/cpp
extern void goToDeepSleep();     // From sleep_manager.h/cpp
