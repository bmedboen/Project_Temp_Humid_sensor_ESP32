// wifi_manager.h

#pragma once
#include <WiFi.h>

/**
 * @brief Initializes WiFi in Access Point (AP) mode.
 * Sets up the AP with predefined SSID and password from config.h.
 * @return true if AP initialized successfully, false otherwise.
 */
bool wifi_manager_init_AP();

/**
 * @brief Connects to WiFi in Station mode using stored credentials.
 * Retrieves SSID and password from Settings Manager.
 * @return true if connected successfully, false otherwise.
 */
bool wifi_manager_connect_STA();

/**
 * @brief Activates Dual Mode: AP + Station.
 * Starts AP immediately, then attempts to connect to home WiFi (if configured).
 * Also initiates mDNS with a unique hostname.
 * @return true if AP started successfully.
 */
bool wifi_manager_start_Interactive_DualMode();

/**
 * @brief Turns off the Wi-Fi radio completely.
 */
void wifi_manager_turnOff();
