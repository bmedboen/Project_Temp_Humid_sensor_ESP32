// external_rtc.cpp
#include "external_rtc.h"
#include "system_logger.h"
#include <Wire.h>

#define LOG_TAG "EXT_RTC"

// Instantiate the global object
ExternalRTCManager RTCManager;

ExternalRTCManager::ExternalRTCManager() {
    _isInitialized = false;
}

bool ExternalRTCManager::begin() {
    if (!HAS_EXTERNAL_RTC) {
        LOG_INFO(LOG_TAG, "External RTC disabled in config.");
        return false;
    }

    // Initialize I2C. Safe to call even if OLED has called it before.
    ::Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!_rtc.begin()) {
        LOG_ERROR(LOG_TAG, "Couldn't find RTC module! Check wiring.");
        _isInitialized = false;
        return false;
    }

    if (_rtc.lostPower()) {
        LOG_WARN(LOG_TAG, "RTC lost power, time is invalid! Battery might be low.");
        // We return true because hardware is present, even if time is wrong.
    }

    LOG_INFO(LOG_TAG, "DS3231 RTC initialized.");
    _isInitialized = true;
    return true;
}

bool ExternalRTCManager::isRunning() {
    return _isInitialized;
}

bool ExternalRTCManager::getTime(struct tm &timeinfo) {
    if (!_isInitialized) return false;

    if (_rtc.lostPower()) {
        LOG_WARN(LOG_TAG, "RTC indicates power loss. Time untrusted.");
        return false; 
    }

    DateTime now = _rtc.now();

    // Convert RTClib DateTime to standard C struct tm
    timeinfo.tm_year = now.year() - 1900; 
    timeinfo.tm_mon  = now.month() - 1;   
    timeinfo.tm_mday = now.day();
    timeinfo.tm_hour = now.hour();
    timeinfo.tm_min  = now.minute();
    timeinfo.tm_sec  = now.second();
    timeinfo.tm_isdst = -1; 

    return true;
}

void ExternalRTCManager::setTime(struct tm timeinfo) {
    if (!_isInitialized) return;

    _rtc.adjust(DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    ));
    LOG_INFO(LOG_TAG, "External RTC time updated.");
}

float ExternalRTCManager::getTemperature() {
    if (!_isInitialized) return NAN;
    return _rtc.getTemperature();
}
