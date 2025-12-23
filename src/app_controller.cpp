// app_controller.cpp

#include "app_controller.h"

// Include all the module headers here, so main doesn't need them
#include "config.h"
#include "data_logger.h"
#include "dht_sensor.h"
#include "sleep_manager.h"
#include "time_manager.h"
#include "web_server.h"
#include "oled_display.h"
#include "wifi_manager.h"
#include "esp_sleep.h"

// --- External Global Variables (from main.cpp) ---
// We need access to these RTC variables to manage logic and sleep times
extern uint64_t time_last_logged_ms; 
extern int deep_sleep_count;
extern "C" uint64_t esp_rtc_get_time_us(); // Internal ESP32 function

// Helper function internal to this file
static void setupHardware() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- Hardware Init ---");
    DHTSensor_init();
    DataLogger_init();
}

/**
 * @brief Activates all peripherals needed for user interaction.
 * Groups OLED, Wi-Fi, and WebServer logic to keep the state machine clean.
 */
 static void startInteractiveServices() {
    Serial.println("Controller: Starting Interactive Services...");

    // 1. Initialize and update OLED
    if (OLEDDisplay_init()) {
        float lastTemp = DataLogger_getLastTemperature();
        float lastHum = DataLogger_getLastHumidity();
        OLEDDisplay_showSensorData(lastTemp, lastHum);
        Serial.println(" - OLED started and updated.");
    }

    // 2. Initialize Wi-Fi
    if (wifi_manager_init_AP()) {
        Serial.println(" - WiFi AP started.");
        
        // 3. Start the Web Server only if WiFi is ready
        if (activateWebServer()) {
             Serial.println(" - WebServer started.");
        } else {
             Serial.println(" - WebServer failed to start!");
        }
    } else {
        Serial.println(" - WiFi AP failed to start! WebServer skipped.");
    }
}

/**
 * @brief Shuts down interactive peripherals to save power before state transition.
 */
 static void stopInteractiveServices() {
    Serial.println("Controller: Stopping Interactive Services...");
    wifi_manager_turnOff();
    OLEDDisplay_turnOff();
}

// --- State Implementations ---
AppState run_state_init() {
    Serial.println("State: INIT");
    setupHardware();
    return STATE_CHECK_WAKEUP; // Directly transition to next state
}

AppState run_state_check_wakeup(bool &stayAwakeFlag) {
    Serial.println("State: CHECK_WAKEUP");
    
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reason: Button Press (EXT0).");
        
        // Debounce
        while(digitalRead(BUTTON_PIN) == LOW) { delay(10); }

        // Display: Showing cached RTC data immediately.
        if (OLEDDisplay_init()) {
            float oldTemp = DataLogger_getLastTemperature();
            float oldHum = DataLogger_getLastHumidity();
            
            OLEDDisplay_showSensorData(oldTemp, oldHum);
        }

        stayAwakeFlag = true;

    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reason: Timer Wakeup.");
        stayAwakeFlag = ENABLE_WEB_SERVER_ON_TIMER_WAKEUP;

    } else {
        Serial.println("Reason: System Reset / First Boot.");
        stayAwakeFlag = true;
    }
    
    return STATE_LOGGING;
}

AppState run_state_logging(bool stayAwakeFlag) {
    Serial.println("State: LOGGING");
    
    uint64_t current_time_ms = esp_rtc_get_time_us() / 1000UL;
    
    float temp = DHTSensor_readTemperature();
    float hum = DHTSensor_readHumidity();

    if (DataLogger_logSensorData(temp, hum)) {
        Serial.println("Logging: Success.");
        time_last_logged_ms = current_time_ms;
    } else {
        Serial.println("Logging: Failed.");
    }

    // Decide next state based on the flag
    if (stayAwakeFlag) {
        return STATE_INTERACTIVE;
    } else {
        return STATE_PREPARE_SLEEP;
    }
}

AppState run_state_interactive() {
    static bool isInitialized = false;

    // 1. Initialization (Run only once)
    if (!isInitialized) {
        startInteractiveServices(); // Call the helper function
        isInitialized = true;
    }

    // 2. Main Loop Logic
    handleWebServerClients();

    // 3. Exit Condition Check
    if (stopWebServerIfIdle()) { 
        Serial.println("Interactive: Timeout reached.");
        stopInteractiveServices(); // Call the helper function
        isInitialized = false; // Reset for next time
        return STATE_PREPARE_SLEEP;
    }

    yield(); // Yield
    return STATE_INTERACTIVE; // Stay in this state
}

AppState run_state_prepare_sleep() {
    Serial.println("State: PREPARE_SLEEP");
    
    uint64_t now_ms = esp_rtc_get_time_us() / 1000UL;
    
    // Calculate sleep duration (returns int64_t now)
    int64_t sleep_us = calculateSleepTime(time_last_logged_ms, now_ms, WAKEUP_OVERHEAD_MS, LOG_INTERVAL_SECONDS * 1000UL);

    deep_sleep_count++;
    
    // The manager handles validation (too short/negative checks).
    goToDeepSleep(sleep_us); 
    
    return STATE_PREPARE_SLEEP; 
}