// oled_display.cpp

#include "oled_display.h"
#include "config.h" // For OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, OLED_I2C_ADDRESS, OLED_RESET_PIN
#include "system_logger.h" // New include for logging

#define LOG_TAG "OLED" // Define a tag for OLEDDisplay module logs

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);

bool OLEDDisplay_init() {
  // Initialize I2C (Wire) for the OLED display
  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    LOG_ERROR(LOG_TAG, "SSD1306 allocation failed");
    return false;
  }

//   // --- TEMPORARY TEST CODE: Show something and pause ---
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0,0);
//   display.println(F("Hello OLED!"));
//   display.println(F("If you see this,"));
//   display.println(F("it's working!"));
//   display.display(); // Show what's in the buffer
//   delay(3000); // *** TEMPORARY DELAY: Keep display on for 3 seconds ***
//   // --- END TEMPORARY TEST CODE ---

  // Initial display setup (no blocking delay here)
  display.display(); // Show initial Adafruit splash screen/buffer content
  display.clearDisplay(); // Clear the buffer immediately
  display.display(); // Update the physical display with the clear buffer
  return true;
}

void OLEDDisplay_clear() {
  display.clearDisplay();
  display.display();
}

void OLEDDisplay_showSensorData(float temperature, float humidity) {
  // Ensure the display is active before drawing
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay(); // Clear the display buffer

  display.setTextSize(2); // Large text
  display.setTextColor(SSD1306_WHITE); // White text

  // --- HEADER: "Last reading:" ---
  display.setTextSize(2); //  Large text for (12x16 pixels per char)
  display.setCursor(0, 0); // Start at top-left
  display.println(F("Last read:")); 

  // Display Temperature
  display.setCursor(0, 16); // Next line
  display.print(F("T: "));
  if (isnan(temperature)) {
    display.print(F("N/A"));
  } else {
    display.print(String(temperature, 1)); // Display with 1 decimal place
  }
  display.println(F(" *C")); // Print unit and move cursor to next line (Y=16+16=32)

  // Display Humidity
  display.setCursor(0, 32); // Move down for humidity
  display.print(F("H: "));
  if (isnan(humidity)) {
    display.print(F("N/A"));
  } else {
    display.print(String(humidity, 1)); // Display with 1 decimal place
  }
  display.println(F(" %")); // Print unit and move cursor to next line (Y=32+16=48)

  display.display(); // Show everything on the display
}

void OLEDDisplay_turnOff() {
  display.clearDisplay();
  display.display(); // Clear the display buffer and update the physical display
  display.ssd1306_command(SSD1306_DISPLAYOFF); // Turn off the display
}
