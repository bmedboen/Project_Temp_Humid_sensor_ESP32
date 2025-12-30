// external_rtc.h
#pragma once

#include <Arduino.h>
#include <RTClib.h> // Requires "RTClib" library by Adafruit
#include "config.h"

/**
 * @brief Class to manage the DS3231 External RTC Module.
 * Encapsulates hardware specific logic using best practices.
 */
class ExternalRTCManager {
private:
    RTC_DS3231 _rtc;       // The driver object
    bool _isInitialized;   // Flag to track initialization status

public:
    ExternalRTCManager();

    /**
     * @brief Initializes I2C and finds the RTC module.
     * @return true if module is found, false otherwise.
     */
    bool begin();

    /**
     * @brief Checks if the RTC is successfully initialized.
     */
    bool isRunning();
    
    /**
     * @brief Reads time from the DS3231.
     * @param timeinfo Reference to a struct tm to store the result.
     * @return true if successful and time is valid, false otherwise.
     */
    bool getTime(struct tm &timeinfo);

    /**
     * @brief Sets the time on the DS3231.
     * @param timeinfo The time to set.
     */
    void setTime(struct tm timeinfo);
    
    /**
     * @brief Reads the internal temperature sensor of the DS3231.
     * @return Temperature in Celsius, or NAN on error.
     */
    float getTemperature();
};

// Global instance available to the application
extern ExternalRTCManager RTCManager;
