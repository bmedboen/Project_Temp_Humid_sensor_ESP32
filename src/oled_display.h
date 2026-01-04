// oled_display.h

#pragma once
#include <Arduino.h>

/**
 * @brief UI Scenes for the state-driven interface
 */
enum DisplayMode {
    OLED_MODE_OFF,
    OLED_MODE_SENSORS,
    OLED_MODE_NETWORK,
    OLED_MODE_MESSAGE
};

/**
 * @brief Initializes the OLED hardware and I2C bus.
 * @return true if successful.
 */
bool OLEDDisplay_init();

/**
 * @brief Sets the current display scene.
 * @param mode The desired DisplayMode.
 */
void OLEDDisplay_setMode(DisplayMode mode);

/**
 * @brief Refreshes the display content based on the active mode.
 * Should be called periodically in INTERACTIVE state.
 */
void OLEDDisplay_refresh();

/**
 * @brief Displays a temporary message with an optional progress bar.
 * @param msg The message string.
 * @param progress Progress percentage (0-100), or -1 to hide the bar.
 */
void OLEDDisplay_showMessage(const char* msg, int progress = -1);

/**
 * @brief Clears the buffer and turns off the display hardware to save power.
 */
void OLEDDisplay_turnOff();
