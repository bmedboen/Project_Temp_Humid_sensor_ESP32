// wifi_manager.cpp

#include "wifi_manager.h"
#include "config.h"
#include "settings_manager.h" // Include the new module
#include "system_logger.h"

#define LOG_TAG "WIFI"

// Timeout for connection attempts
const unsigned long WIFI_SETUP_TIMEOUT_MS = 5000; 

bool wifi_manager_init_AP() {
    LOG_INFO(LOG_TAG, "Initializing in AP mode...");
    
    WiFi.mode(WIFI_AP);
    // AP credentials can also be moved to config/settings later if desired
    WiFi.softAP(AP_SSID, AP_PASSWORD); 
    
    LOG_INFO(LOG_TAG, "Waiting for AP IP address...");
    
    unsigned long startTime = ::millis();
    while (WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        if (::millis() - startTime > WIFI_SETUP_TIMEOUT_MS) {
            LOG_ERROR(LOG_TAG, "Wi-Fi AP setup failed! Timed out.");
            return false;
        }
        ::delay(100);
    }
    
    IPAddress IP = WiFi.softAPIP();
    LOG_INFO(LOG_TAG, "AP IP address: %s", IP.toString().c_str());
    return true; 
}

bool wifi_manager_connect_STA() {
    // 1. Retrieve credentials from Settings Manager (Handles NVS vs Secrets logic)
    String ssid = Settings.getWifiSSID();
    String pass = Settings.getWifiPassword();

    // 2. Validate that we have an SSID to connect to
    if (ssid == "") {
        LOG_WARN(LOG_TAG, "Skipping WiFi STA: No SSID configured.");
        return false;
    }

    LOG_INFO(LOG_TAG, "Connecting to WiFi: %s", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = ::millis();
    while (WiFi.status() != WL_CONNECTED) {
        // Use the timeout from config.h
        if (::millis() - start > WIFI_CONNECT_TIMEOUT_MS) { 
            LOG_ERROR(LOG_TAG, "Failed to connect to WiFi (Timeout).");
            return false;
        }
        ::delay(500);
    }

    LOG_INFO(LOG_TAG, "WiFi Connected! IP: %s", WiFi.localIP().toString().c_str());
    return true;
}

void wifi_manager_turnOff() {
    LOG_INFO(LOG_TAG, "Turning off Wi-Fi radio...");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}
