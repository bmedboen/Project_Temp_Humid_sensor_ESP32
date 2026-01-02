// wifi_manager.cpp

#include "wifi_manager.h"
#include "config.h"
#include "settings_manager.h" 
#include "system_logger.h"
#include <ESPmDNS.h> 

#define LOG_TAG "WIFI"

// --- PRIVATE HELPER FUNCTIONS ---

/**
 * @brief Internal helper to start the Access Point.
 * Contains the logic for softAP configuration and waiting for IP.
 */
static bool _startAP_internal() {
    // We use the fixed AP credentials from config.h
    WiFi.softAP(AP_SSID, AP_PASSWORD); 
    
    // Wait for IP address assignment (usually very fast for AP)
    unsigned long start = ::millis(); // Use :: to access global C function
    while (WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        if (::millis() - start > 2000) { // Short timeout (2s) is enough for AP
            LOG_ERROR(LOG_TAG, "AP Setup Failed.");
            return false;
        }
        ::delay(100); // Use :: to access global C function
    }

    LOG_INFO(LOG_TAG, "AP Started. IP: %s", WiFi.softAPIP().toString().c_str());
    return true;
}

/**
 * @brief Internal helper to connect to a Router (Station).
 * @param timeout_ms How long to block waiting for connection.
 * @return true if connected, false if timeout (but connection might continue in bg).
 */
static bool _connectSTA_internal(unsigned long timeout_ms) {
    String ssid = Settings.getWifiSSID();
    String pass = Settings.getWifiPassword();

    // Basic validation
    if (ssid == "") {
        if (timeout_ms > 2000) {
            LOG_WARN(LOG_TAG, "Skipping STA: No custom SSID configured.");
        }
        return false;
    }

    LOG_INFO(LOG_TAG, "Connecting to: %s", ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = ::millis(); // Use :: to access global C function
    while (WiFi.status() != WL_CONNECTED) {
        // Check timeout
        if (::millis() - start > timeout_ms) {
            LOG_INFO(LOG_TAG, "STA Wait Timeout (waited %lu ms). Continuing in background...", timeout_ms);
            return false;
        }
        ::delay(100); // Use :: to access global C function
    }

    LOG_INFO(LOG_TAG, "STA Connected! IP: %s", WiFi.localIP().toString().c_str());

    // --- Unique mDNS Hostname Logic ---
    
    // 1. Get Base Name from config
    String hostname = MDNS_HOSTNAME;

    // 2. Get MAC Address to make the name unique
    String mac = WiFi.macAddress();
    // Format is "AA:BB:CC:DD:EE:FF". We remove colons.
    mac.replace(":", ""); 
    
    // 3. Append the last 4 characters of the MAC
    String suffix = mac.substring(8); 
    hostname += "-" + suffix;
    hostname.toLowerCase(); // mDNS standards prefer lowercase

    // 4. Start mDNS
    if (MDNS.begin(hostname.c_str())) {
        LOG_INFO(LOG_TAG, "mDNS responder started! URL: http://%s.local", hostname.c_str());
    } else {
        LOG_ERROR(LOG_TAG, "Error setting up mDNS responder!");
    }
    // -------------------------------------------

    return true;
}


// --- PUBLIC FUNCTIONS ---

bool wifi_manager_init_AP() {
    LOG_INFO(LOG_TAG, "Setting mode: AP Only");
    WiFi.mode(WIFI_AP);
    return _startAP_internal();
}

bool wifi_manager_connect_STA() {
    LOG_INFO(LOG_TAG, "Setting mode: Station Only");
    WiFi.mode(WIFI_STA);
    
    // For NTP sync, we wait the full timeout (critical operation)
    return _connectSTA_internal(WIFI_CONNECT_TIMEOUT_MS);
}

bool wifi_manager_start_Interactive_DualMode() {
    LOG_INFO(LOG_TAG, "Setting mode: Dual (AP + Station)");
    
    // 1. Set Dual Mode (Critical step!)
    WiFi.mode(WIFI_AP_STA);

    // 2. Start AP (Must work for user interaction immediately)
    bool apSuccess = _startAP_internal();

    // 3. Try to connect to Home WiFi
    // We use a SHORT timeout (3s). If it fails, it keeps trying in the background
    // while the user uses the AP.
    _connectSTA_internal(3000); 

    // Return true if AP works (Primary interface)
    return apSuccess;
}

void wifi_manager_turnOff() {
    LOG_INFO(LOG_TAG, "Turning off Wi-Fi radio...");
    WiFi.disconnect(true);  // true = turn off WiFi radio
    WiFi.mode(WIFI_OFF);
}
