// time_manager.cpp
#include "time_manager.h"
#include "external_rtc.h" 
#include "wifi_manager.h" 
#include "system_logger.h"
#include "config.h"
#include <sys/time.h> 

#define LOG_TAG "TIME"

// --- Helper: Update Internal System Clock ---
static void applyTimeToSystem(struct tm timeinfo) {
    struct timeval tv;
    tv.tv_sec = ::mktime(&timeinfo); 
    tv.tv_usec = 0;
    // Using :: to ensure we call the global POSIX function
    ::settimeofday(&tv, NULL);       
}

// --- Helper: Perform NTP Sync ---
static bool performNtpSync() {
    LOG_INFO(LOG_TAG, "Configuring NTP (Server: %s)...", NTP_SERVER);
    
    // Native ESP32 function
    ::configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    struct tm timeinfo;
    unsigned long start = ::millis();
    LOG_INFO(LOG_TAG, "Waiting for NTP time sync...");
    
    while (!::getLocalTime(&timeinfo)) {
        if (::millis() - start > NTP_TIMEOUT_MS) {
            LOG_ERROR(LOG_TAG, "NTP Sync Timeout!");
            return false;
        }
        ::delay(100);
    }
    return true;
}

// --- Public Functions ---

bool TimeManager_startAndSync() {
    struct tm timeinfo;
    bool internalValid = false;
    bool externalValid = false;

    // 1. Initialize External RTC Hardware
    if (RTCManager.begin()) {
        // Check if External RTC has valid time
        if (RTCManager.getTime(timeinfo)) {
            // Check if year is valid (> 2024)
            if ((timeinfo.tm_year + 1900) > 2024) {
                externalValid = true;
            } else {
                 LOG_WARN(LOG_TAG, "External RTC found, but time is invalid/old.");
            }
        }
    }

    // 2. Check Internal ESP32 Clock (Maintained in Deep Sleep)
    if (::getLocalTime(&timeinfo)) {
        if ((timeinfo.tm_year + 1900) > 2024) {
             internalValid = true;
             LOG_INFO(LOG_TAG, "Internal system clock is valid.");
             return true; 
        }
    }

    // 3. Logic Decision Tree
    
    // CASE A: Internal lost (Reset), External OK.
    if (!internalValid && externalValid) {
        LOG_INFO(LOG_TAG, "Internal clock lost. Restoring from External RTC...");
        RTCManager.getTime(timeinfo); 
        applyTimeToSystem(timeinfo);
        return true;
    }

    // CASE B: Both Invalid (Dead battery/First boot). Use NTP.
    if (!internalValid && !externalValid) {
        LOG_WARN(LOG_TAG, "No valid time source found. Starting WiFi for NTP...");
        
        if (wifi_manager_connect_STA()) {
            if (performNtpSync()) {
                LOG_INFO(LOG_TAG, "NTP Sync Success.");
                
                if (::getLocalTime(&timeinfo)) {
                    // Sync NTP time to External RTC for future use
                    RTCManager.setTime(timeinfo); 
                    LOG_INFO(LOG_TAG, "Updated External RTC with NTP time.");
                }
                
                wifi_manager_turnOff();
                return true;
            }
            wifi_manager_turnOff();
        }
    }

    LOG_ERROR(LOG_TAG, "Failed to synchronize time.");
    return false;
}

String getFormattedTime() {
    struct tm timeinfo;
    if (!::getLocalTime(&timeinfo)) {
        return "Time Not Set";
    }
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStr);
}

bool TimeManager_isTimeSet() {
    struct tm timeinfo;
    if (!::getLocalTime(&timeinfo)) return false;
    return (timeinfo.tm_year + 1900) > 2024;
}
