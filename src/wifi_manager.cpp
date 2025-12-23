// wifi_manager.cpp

#include "wifi_manager.h"
#include "config.h"
#include "Arduino.h"

// Define a timeout for Wi-Fi setup
const unsigned long WIFI_SETUP_TIMEOUT_MS = 5000; // 5 seconds

bool wifi_manager_init_AP() {
    Serial.println("WiFi Manager: Initializing in AP mode...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD); 
    
    Serial.print("WiFi Manager: Waiting for AP IP address...");
    
    unsigned long startTime = millis();
    while (WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        if (millis() - startTime > WIFI_SETUP_TIMEOUT_MS) {
            Serial.println("\nWiFi Manager: Wi-Fi setup failed! Timed out.");
            return false; // Return false on failure
        }
        Serial.print(".");
        delay(100);
    }
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("\nWiFi Manager: AP IP address: ");
    Serial.println(IP);
    return true; // Return true on success
}

void wifi_manager_turnOff() {
    Serial.println("WiFi Manager: Turning off Wi-Fi radio...");
    
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}
