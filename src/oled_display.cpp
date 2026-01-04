// oled_display.cpp

#include "oled_display.h"
#include "config.h"
#include "system_logger.h"
#include "data_logger.h"
#include "wifi_manager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LOG_TAG "OLED"

// Internal static variables
static Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
static DisplayMode currentMode = OLED_MODE_OFF;
static String currentMessage = "";
static int currentProgress = -1;

bool OLEDDisplay_init() {
    // Initialize I2C with explicit pins from config/pinout
    if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
        LOG_ERROR(LOG_TAG, "I2C initialization failed");
        return false;
    }

    // Initialize SSD1306
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        LOG_ERROR(LOG_TAG, "SSD1306 allocation failed");
        return false;
    }

    display.clearDisplay();
    display.display();
    return true;
}

void OLEDDisplay_setMode(DisplayMode mode) {
    if (mode == OLED_MODE_OFF) {
        OLEDDisplay_turnOff();
        return;
    }
    currentMode = mode;
    // Ensure display hardware is awake
    display.ssd1306_command(SSD1306_DISPLAYON);
    OLEDDisplay_refresh();
}

void OLEDDisplay_showMessage(const char* msg, int progress) {
    currentMessage = msg;
    currentProgress = progress;
    currentMode = OLED_MODE_MESSAGE;
    OLEDDisplay_refresh();
}

// --- Internal Scene Drawing Functions ---

static void drawHeader(const char* title) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(title);
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
}

static void drawSensorScene() {
    drawHeader("ENVIRONMENT LOG");
    
    // Pull data directly from DataLogger
    float t = DataLogger_getLastTemperature();
    float h = DataLogger_getLastHumidity();
    String timestamp = DataLogger_getLastLogTime();

    display.setCursor(0, 16);
    display.setTextSize(2);
    
    if (isnan(t)) display.println("T: --.- C");
    else display.printf("T: %.1f C\n", t);
    
    if (isnan(h)) display.println("H: --.- %");
    else display.printf("H: %.1f %%", h);

    display.setCursor(0, 55);
    display.setTextSize(1);
    display.printf("@ %s", timestamp.c_str());
}

static void drawNetworkScene() {
    drawHeader("NETWORK STATUS");
    display.setTextSize(1);
    display.setCursor(0, 18);

    // Pull status directly from WiFi library
    if (WiFi.status() == WL_CONNECTED) {
        display.println("Status: Connected");
        display.printf("IP:  %s\n", WiFi.localIP().toString().c_str());
        display.printf("URL: %s.local", wifi_manager_get_hostname().c_str());
    } else {
        display.println("Status: Connecting...");
        display.printf("AP:  %s\n", AP_SSID);
        display.printf("IP:  %s", WiFi.softAPIP().toString().c_str());
    }
}

static void drawMessageScene() {
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println(currentMessage);

    if (currentProgress >= 0) {
        int barWidth = map(currentProgress, 0, 100, 0, 100);
        display.drawRect(14, 45, 100, 8, SSD1306_WHITE);
        display.fillRect(14, 45, barWidth, 8, SSD1306_WHITE);
    }
}

void OLEDDisplay_refresh() {
    if (currentMode == OLED_MODE_OFF) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    switch (currentMode) {
        case OLED_MODE_SENSORS: drawSensorScene();  break;
        case OLED_MODE_NETWORK: drawNetworkScene(); break;
        case OLED_MODE_MESSAGE: drawMessageScene(); break;
        default: break;
    }
    display.display();
}

void OLEDDisplay_turnOff() {
    currentMode = OLED_MODE_OFF;
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    LOG_INFO(LOG_TAG, "Display turned off (Hardware Command)");
}
