// wifi_manager.cpp

#include "wifi_manager.h"
#include "config.h"
#include "Arduino.h"
#include "system_logger.h" // New include for logging

#define LOG_TAG "WIFI" // Define a tag for WiFi Manager module logs

// Define a timeout for Wi-Fi setup
const unsigned long WIFI_SETUP_TIMEOUT_MS = 5000; // 5 seconds

bool wifi_manager_init_AP() {
    LOG_INFO(LOG_TAG, "Initializing in AP mode...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD); 
    
    LOG_INFO(LOG_TAG, "Waiting for AP IP address...");
    
    unsigned long startTime = millis();
    while (WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        if (millis() - startTime > WIFI_SETUP_TIMEOUT_MS) {
            LOG_ERROR(LOG_TAG, "Wi-Fi AP setup failed! Timed out.");
            return false; // Return false on failure
        }
        delay(100); // Changed from Serial.print(".")
    }
    
    IPAddress IP = WiFi.softAPIP();
    LOG_INFO(LOG_TAG, "AP IP address: %s", IP.toString().c_str());
    return true; // Return true on success
}

bool wifi_manager_connect_STA() {
    LOG_INFO(LOG_TAG, "Connecting to WiFi: %s", WIFI_STA_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);

    unsigned long start = ::millis();
    while (WiFi.status() != WL_CONNECTED) {
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
