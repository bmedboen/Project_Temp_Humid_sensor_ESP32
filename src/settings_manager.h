
// settings_manager.h
#pragma once
#include <Arduino.h>
#include <Preferences.h>

/**
 * @brief Manages persistent storage (NVS) and configuration.
 * Acts as a wrapper around the ESP32 Preferences library.
 * Implements a fallback mechanism using secrets.h if NVS is empty.
 */
class SettingsManager {
private:
    Preferences _prefs;         // The underlying NVS object
    bool _isInitialized;        // Flag to track if NVS opened successfully
    const char* _namespace = "app_config"; // The NVS partition name

public:
    SettingsManager(); // Constructor

    /**
     * @brief Opens the NVS storage. Must be called in setup().
     */
    void begin();
    
    // --- Getters ---
    // These return the value from NVS. 
    // If NVS is empty, they return the defaults from secrets.h.
    String getWifiSSID();
    String getWifiPassword();
    
    // --- Setters ---
    // These save new values to NVS (Persistent across reboots).
    void saveWifiCredentials(String ssid, String password);
    
    // --- Utilities ---
    // Clears stored data to force a reversion to factory defaults.
    void clearWifiCredentials(); 
};

// Global instance available to the entire application
extern SettingsManager Settings;
