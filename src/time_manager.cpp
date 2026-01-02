// time_manager.cpp

#include "time_manager.h"
#include "external_rtc.h" 
#include "wifi_manager.h" 
#include "system_logger.h"
#include "config.h"
#include <sys/time.h> // For settimeofday

#define LOG_TAG "TIME"

// --- PRIVATE HELPER FUNCTIONS ---

/**
 * @brief Centralized validation logic.
 * Defines what constitutes a "valid" time for this system.
 */
static bool isValidYear(const struct tm *t) {
    if (t == nullptr) return false;
    return (t->tm_year + 1900) > 2024;
}

/**
 * @brief Performs the NTP synchronization sequence.
 * Assumes WiFi is already connected.
 */
static bool performNtpSync() {
    LOG_INFO(LOG_TAG, "Configuring NTP (Server: %s)...", NTP_SERVER);
    
    // Trigger the background NTP sync using ESP32 native function
    ::configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    unsigned long start = ::millis();
    LOG_INFO(LOG_TAG, "Waiting for NTP time sync...");
    
    // Wait until the system time becomes valid
    while (!TimeManager_isTimeSet()) {
        if (::millis() - start > NTP_TIMEOUT_MS) {
            LOG_ERROR(LOG_TAG, "NTP Sync Timeout!");
            return false;
        }
        ::delay(100);
    }
    return true;
}

// --- PUBLIC FUNCTIONS ---

void TimeManager_setTime(struct tm t, bool updateRTC) {
    
    // 1. Always update Internal ESP32 System Clock
    // Convert struct tm to time_t (seconds since epoch)
    time_t timeVal = ::mktime(&t);
    struct timeval tv = { timeVal, 0 };
    
    // Apply to OS
    ::settimeofday(&tv, NULL);
    
    LOG_INFO(LOG_TAG, "Internal System Clock set.");

    // 2. Conditionally update External RTC
    // We only write to RTC when the source is external (NTP or Web).
    // Writing to RTC constantly (reading then writing back) resets the 
    // internal prescaler, causing the clock to drift/lag over time.
    if (updateRTC) {
        if (RTCManager.isRunning()) {
            RTCManager.setTime(t);
            // RTCManager handles its own success logging
        } else {
            LOG_WARN(LOG_TAG, "Cannot update External RTC (Not found).");
        }
    }
}

bool TimeManager_isTimeSet() {
    struct tm timeinfo;
    if (!::getLocalTime(&timeinfo)) return false;
    // Reuse centralized validation logic
    return isValidYear(&timeinfo);
}

bool TimeManager_startAndSync() {
    struct tm timeinfo;
    bool rtcFound = false;

    // --- PRIORITY 1: EXTERNAL RTC (The Master Clock) ---
    // DS3231 is a TCXO (Temperature Compensated) and is highly accurate.
    // We always attempt to sync from it first to correct any drift 
    // in the ESP32's internal RC oscillator.
    if (RTCManager.begin()) {
        rtcFound = true;
        if (RTCManager.getTime(timeinfo)) {
            if (isValidYear(&timeinfo)) {
                LOG_INFO(LOG_TAG, "Syncing system time from External RTC.");
                
                // IMPORTANT: Pass 'false' to prevent writing back to RTC.
                // This avoids the "prescaler reset" drift issue.
                TimeManager_setTime(timeinfo, false); 
                
                return true; 
            } else {
                 LOG_WARN(LOG_TAG, "External RTC found, but time is invalid/old.");
            }
        }
    } else {
        LOG_WARN(LOG_TAG, "External RTC not found.");
    }

    // --- PRIORITY 2: INTERNAL CLOCK (Fallback) ---
    // If RTC failed (e.g., dead battery), check if ESP32 internal clock 
    // is still valid (maintained during Deep Sleep).
    if (TimeManager_isTimeSet()) {
         LOG_INFO(LOG_TAG, "Using internal system clock (No RTC sync).");
         return true; 
    }

    // --- PRIORITY 3: NTP (Last Resort) ---
    // Neither RTC nor Internal clock is valid. We must connect to WiFi.
    LOG_WARN(LOG_TAG, "No valid time source. Starting WiFi for NTP...");
    
    if (wifi_manager_connect_STA()) {
        if (performNtpSync()) {
            LOG_INFO(LOG_TAG, "NTP Sync Success.");
            
            // Now that internal time is correct (thanks to NTP),
            // we grab it and push it through our Master Function.
            if (::getLocalTime(&timeinfo)) {
                // Pass 'true' because we WANT to save this fresh network time 
                // to the External RTC for future boot cycles.
                if (rtcFound) {
                    TimeManager_setTime(timeinfo, true);
                    LOG_INFO(LOG_TAG, "Time synced and saved to External RTC.");
                }
            }
            
            wifi_manager_turnOff();
            return true;
        }
        wifi_manager_turnOff();
    }

    LOG_ERROR(LOG_TAG, "CRITICAL: Failed to synchronize time.");
    return false;
}

String getFormattedTime() {
    struct tm timeinfo;
    if (!::getLocalTime(&timeinfo)) {
        return "Time Not Set";
    }
    char timeStr[30];
    ::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStr);
}
