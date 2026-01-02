// settings_manager.cpp

#include "settings_manager.h"
#include "system_logger.h"

// --- FALLBACK HANDLING ---
// We check if secrets.h exists. If code is downloaded from GitHub
// without secrets.h, it will still compile but with empty defaults.
#if __has_include("secrets.h")
    #include "secrets.h"
#endif

// Define defaults if secrets.h is missing
#ifndef DEFAULT_WIFI_SSID
    #define DEFAULT_WIFI_SSID ""
#endif

#ifndef DEFAULT_WIFI_PASS
    #define DEFAULT_WIFI_PASS ""
#endif

#define LOG_TAG "SETTINGS"

// Instantiate the global object
SettingsManager Settings;

SettingsManager::SettingsManager() {
    _isInitialized = false;
}

void SettingsManager::begin() {
    // Open NVS in read/write mode (false)
    if (_prefs.begin(_namespace, false)) { 
        _isInitialized = true;
        LOG_INFO(LOG_TAG, "NVS Storage initialized.");
    } else {
        LOG_ERROR(LOG_TAG, "Failed to initialize NVS!");
    }
}

String SettingsManager::getWifiSSID() {
    // Safety check
    if (!_isInitialized) return String(DEFAULT_WIFI_SSID);
    
    // Logic: Get string from NVS. If key "ssid" doesn't exist, return DEFAULT_WIFI_SSID.       
    if (!_prefs.isKey("ssid")) {
        return String(DEFAULT_WIFI_SSID);
    }

    String ssid = _prefs.getString("ssid", DEFAULT_WIFI_SSID);
    
    // Logging for debugging (Optional: remove if too verbose)
    if (ssid == DEFAULT_WIFI_SSID && String(DEFAULT_WIFI_SSID) != "") {
        LOG_INFO(LOG_TAG, "Using Factory Default SSID (from secrets.h).");
    } else if (ssid != "") {
        LOG_INFO(LOG_TAG, "Using Custom SSID (from NVS).");
    } else {
        LOG_WARN(LOG_TAG, "No WiFi SSID configured! (NVS empty & No Default)");
    }
    return ssid;
}

String SettingsManager::getWifiPassword() {
    if (!_isInitialized) return String(DEFAULT_WIFI_PASS);
    
    // FIX: Samme sjekk for passord
    if (!_prefs.isKey("pass")) {
        return String(DEFAULT_WIFI_PASS);
    }

    return _prefs.getString("pass", DEFAULT_WIFI_PASS);
}

void SettingsManager::saveWifiCredentials(String ssid, String password) {
    if (!_isInitialized) {
        LOG_ERROR(LOG_TAG, "Cannot save: NVS not initialized.");
        return;
    }
    
    if (ssid.length() > 0) {
        _prefs.putString("ssid", ssid);
        _prefs.putString("pass", password);
        LOG_INFO(LOG_TAG, "New credentials saved to NVS. Reboot to apply.");
    }
}

void SettingsManager::clearWifiCredentials() {
    if (!_isInitialized) return;
    _prefs.remove("ssid");
    _prefs.remove("pass");
    LOG_INFO(LOG_TAG, "NVS cleared. Reverting to factory defaults.");
}
