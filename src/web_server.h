// web_server.h
#pragma once

#include <Arduino.h>

/**
 * @brief Activates the Async Web Server.
 * Checks WiFi state and starts the server.
 * @return true if server started successfully or is already running.
 */
bool activateWebServer();

/**
 * @brief Kept for backward compatibility. 
 * AsyncWebServer handles clients automatically in the background, so this is empty.
 */
void handleWebServerClients();

/**
 * @brief Checks if the server object is currently running.
 * @return true if running.
 */
bool isWebServerActive();

/**
 * @brief Checks if the inactivity timeout has been reached.
 * If true, stops the server to save power.
 * @return true if the server was stopped.
 */
bool stopWebServerIfIdle();
