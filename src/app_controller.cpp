// app_controller.cpp

#include "app_controller.h"

// Modules
#include "config.h"
#include "system_logger.h" 
#include "data_logger.h"
#include "dht_sensor.h"
#include "sleep_manager.h"
#include "time_manager.h"
#include "web_server.h"
#include "oled_display.h"
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "settings_manager.h"

#define LOG_TAG "CONTROLLER" // Define a tag for DataLogger module logs

// --- External Global Variables ---
extern uint64_t time_last_logged_ms; 
extern int deep_sleep_count;
extern "C" uint64_t esp_rtc_get_time_us(); 

// --- Helper Functions ---
static void setupHardware() {
    
    LOG_INFO(LOG_TAG, "Hardware: Setting up hardware...");
    
    // 1. Initialize NVS Settings First (Critical for WiFi)
    Settings.begin();

    // 2. Initialize Sensors and Storage
    if(DHTSensor_init()) {
        LOG_INFO(LOG_TAG, "Hardware: DHT Sensor initialized.");
    } else {
        LOG_ERROR(LOG_TAG, "Hardware: DHT Sensor failed!");
    }

    if(DataLogger_init()) {
        LOG_INFO(LOG_TAG, "Hardware: LittleFS initialized.");
    } else {
        LOG_ERROR(LOG_TAG, "Hardware: LittleFS Mount Failed!");
    }
}

static void startInteractiveServices() {
    LOG_INFO(LOG_TAG, "Starting Interactive Services...");

    // 1. Initialize and update OLED
    if (OLEDDisplay_init()) {
        float lastTemp = DataLogger_getLastTemperature();
        float lastHum = DataLogger_getLastHumidity();
        OLEDDisplay_showSensorData(lastTemp, lastHum);
        LOG_INFO(LOG_TAG, "OLED active.");
    }

    // 2. Initialize Wi-Fi
    if (wifi_manager_start_Interactive_DualMode()) {
        LOG_INFO(LOG_TAG, "WiFi Dual Mode started.");
        
        // 3. Start the Web Server
        if (activateWebServer()) {
             LOG_INFO(LOG_TAG, "WebServer active.");
        } else {
             LOG_ERROR(LOG_TAG, "WebServer failed to start.");
        }
    } else {
        LOG_ERROR(LOG_TAG, "WiFi failed! WebServer skipped.");
    }
}

static void stopInteractiveServices() {
    LOG_INFO(LOG_TAG, "Stopping Interactive Services...");
    wifi_manager_turnOff();
    OLEDDisplay_turnOff();
}

// --- State Implementations ---
AppState run_state_init() {
    setupHardware();    
    
    // Logic: Internal -> External RTC -> NTP handled entirely by TimeManager
    if (TimeManager_startAndSync()) {
        LOG_INFO(LOG_TAG, "Time synchronization complete.");
    } else {
        LOG_ERROR(LOG_TAG, "CRITICAL: System running without correct time. Please update manually.");
        // We continue anyway, allowing manual override later via Web Interface
    }

    return STATE_CHECK_WAKEUP; 
}

AppState run_state_check_wakeup(bool &stayAwakeFlag) {    
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        LOG_INFO(LOG_TAG, "Wakeup reason: Button Press (EXT0)");
        
        // Show cached data immediately for responsiveness
        if (OLEDDisplay_init()) {
            float oldTemp = DataLogger_getLastTemperature();
            float oldHum = DataLogger_getLastHumidity();
            OLEDDisplay_showSensorData(oldTemp, oldHum);
        }
        
        delay(50); // Debounce
        stayAwakeFlag = true;

    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        LOG_INFO(LOG_TAG, "Wakeup reason: Timer (Scheduled Log)");
        stayAwakeFlag = ENABLE_WEB_SERVER_ON_TIMER_WAKEUP;

    } else {
        LOG_INFO(LOG_TAG, "Wakeup reason: System Reset / First Boot");
        
        // Show N/A on OLED
        if (OLEDDisplay_init()) {
            OLEDDisplay_showSensorData(NAN, NAN);
        }
        stayAwakeFlag = true;
    }
    
    return STATE_LOGGING;
}

AppState run_state_logging(bool stayAwakeFlag) {
    
    uint64_t current_time_ms = esp_rtc_get_time_us() / 1000UL;
    
    float temp = DHTSensor_readTemperature();
    float hum = DHTSensor_readHumidity();

    LOG_DEBUG(LOG_TAG, "Raw Read -> T: %.1f, H: %.1f", temp, hum);

    if (DataLogger_logSensorData(temp, hum)) {
        LOG_INFO(LOG_TAG, "Logged successfully. T: %.1f C, H: %.1f %%", temp, hum);

        time_last_logged_ms = current_time_ms;
    } else {
        LOG_ERROR(LOG_TAG, "Failed to write data to file!");
    }

    if (stayAwakeFlag) {
        return STATE_INTERACTIVE;
    } else {
        return STATE_PREPARE_SLEEP;
    }
}

AppState run_state_interactive() {
    static bool isInitialized = false;

    if (!isInitialized) {
        startInteractiveServices(); 
        isInitialized = true;
    }

    if (stopWebServerIfIdle()) { 
        LOG_INFO(LOG_TAG, "Inactivity timeout reached.");
        stopInteractiveServices(); 
        isInitialized = false; 
        return STATE_PREPARE_SLEEP;
    }

    delay(50); // Small delay to yield CPU 

    return STATE_INTERACTIVE; 
}

AppState run_state_prepare_sleep() {    
    uint64_t now_ms = esp_rtc_get_time_us() / 1000UL;
    
    int64_t sleep_us = calculateSleepTime(time_last_logged_ms, now_ms, WAKEUP_OVERHEAD_MS, LOG_INTERVAL_SECONDS * 1000UL);

    LOG_INFO(LOG_TAG, "Deep sleep cycles so far: %d", deep_sleep_count);
    deep_sleep_count++;
    
    goToDeepSleep(sleep_us); 
    
    return STATE_PREPARE_SLEEP; 
}
