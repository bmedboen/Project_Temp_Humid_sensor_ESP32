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

#define LOG_TAG "DATALOG" // Define a tag for DataLogger module logs

// --- External Global Variables ---
extern uint64_t time_last_logged_ms; 
extern int deep_sleep_count;
extern "C" uint64_t esp_rtc_get_time_us(); 

// --- Helper Functions ---

static void setupHardware() {
    // Initialize Logger (starts Serial)
    Logger_Init();
    
    LOG_INFO("Hardware", "Initializing components...");
    
    if(DHTSensor_init()) {
        LOG_INFO("Hardware", "DHT Sensor initialized.");
    } else {
        LOG_ERROR("Hardware", "DHT Sensor failed!");
    }

    if(DataLogger_init()) {
        LOG_INFO("Hardware", "LittleFS initialized.");
    } else {
        LOG_ERROR("Hardware", "LittleFS Mount Failed!");
    }
}

static void startInteractiveServices() {
    LOG_INFO("Controller", "Starting Interactive Services...");

    // 1. Initialize and update OLED
    if (OLEDDisplay_init()) {
        float lastTemp = DataLogger_getLastTemperature();
        float lastHum = DataLogger_getLastHumidity();
        OLEDDisplay_showSensorData(lastTemp, lastHum);
        LOG_INFO("Controller", "OLED active.");
    }

    // 2. Initialize Wi-Fi
    if (wifi_manager_init_AP()) {
        LOG_INFO("Controller", "WiFi AP started.");
        
        // 3. Start the Web Server
        if (activateWebServer()) {
             LOG_INFO("Controller", "WebServer active.");
        } else {
             LOG_ERROR("Controller", "WebServer failed to start.");
        }
    } else {
        LOG_ERROR("Controller", "WiFi AP failed! WebServer skipped.");
    }
}

static void stopInteractiveServices() {
    LOG_INFO("Controller", "Stopping Interactive Services...");
    wifi_manager_turnOff();
    OLEDDisplay_turnOff();
}

// --- State Implementations ---

AppState run_state_init() {
    setupHardware();
    LOG_INFO("State", "--- Boot Complete ---");
    return STATE_CHECK_WAKEUP; 
}

AppState run_state_check_wakeup(bool &stayAwakeFlag) {
    LOG_DEBUG("State", "Entering CHECK_WAKEUP");
    
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        LOG_INFO("Wakeup", "Reason: Button Press (EXT0)");
        
        // Show cached data immediately for responsiveness
        if (OLEDDisplay_init()) {
            float oldTemp = DataLogger_getLastTemperature();
            float oldHum = DataLogger_getLastHumidity();
            OLEDDisplay_showSensorData(oldTemp, oldHum);
            LOG_DEBUG("Wakeup", "Cached data displayed.");
        }
        
        delay(50); // Debounce
        stayAwakeFlag = true;

    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        LOG_INFO("Wakeup", "Reason: Timer (Scheduled Log)");
        stayAwakeFlag = ENABLE_WEB_SERVER_ON_TIMER_WAKEUP;

    } else {
        LOG_INFO("Wakeup", "Reason: System Reset / First Boot");
        
        // Show N/A on OLED
        if (OLEDDisplay_init()) {
            OLEDDisplay_showSensorData(NAN, NAN);
        }
        stayAwakeFlag = true;
    }
    
    return STATE_LOGGING;
}

AppState run_state_logging(bool stayAwakeFlag) {
    LOG_DEBUG("State", "Entering LOGGING");
    
    uint64_t current_time_ms = esp_rtc_get_time_us() / 1000UL;
    
    float temp = DHTSensor_readTemperature();
    float hum = DHTSensor_readHumidity();

    LOG_DEBUG("Sensor", "Raw Read -> T: %.1f, H: %.1f", temp, hum);

    if (DataLogger_logSensorData(temp, hum)) {
        LOG_INFO("DataLogger", "Logged successfully. T: %.1f C, H: %.1f %%", temp, hum);
        // Critical: Update timestamp
        time_last_logged_ms = current_time_ms;
    } else {
        LOG_ERROR("DataLogger", "Failed to write data to file!");
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

    handleWebServerClients();

    if (stopWebServerIfIdle()) { 
        LOG_INFO("Interactive", "Inactivity Timeout reached.");
        stopInteractiveServices(); 
        isInitialized = false; 
        return STATE_PREPARE_SLEEP;
    }

    yield(); 
    return STATE_INTERACTIVE; 
}

AppState run_state_prepare_sleep() {
    LOG_DEBUG("State", "Entering PREPARE_SLEEP");
    
    uint64_t now_ms = esp_rtc_get_time_us() / 1000UL;
    
    int64_t sleep_us = calculateSleepTime(time_last_logged_ms, now_ms, WAKEUP_OVERHEAD_MS, LOG_INTERVAL_SECONDS * 1000UL);

    LOG_INFO("Sleep", "Deep sleep cycles so far: %d", deep_sleep_count);
    deep_sleep_count++;
    
    goToDeepSleep(sleep_us); 
    
    return STATE_PREPARE_SLEEP; 
}
