// web_server.cpp

#include "web_server.h" // Include its own header file

#include <WiFi.h>
#include <sys/time.h>   // For settimeofday
#include <FS.h> 
#include <LittleFS.h> 

#include "config.h"
#include "data_logger.h" 
#include "time_manager.h" // For getFormattedTime()
#include "system_logger.h" // New include for logging

#define LOG_TAG "WEB" // Define a tag for WebServer module logs

// --- Global variables (definitions from web_server.h) ---
WebServer server(WEBSERVER_PORT); // Define the WebServer object (port 80)
static uint64_t webServerLastActivityTime = 0;
static bool isServerRunning = false;

// --- PRIVATE Web Server Handler Functions ---
// Helper functions
static void setupWebServerRoutes_internal();
static void startWebServer_internal(); 
static void resetWebServerActivityTimer_internal();
static bool isWebServerTimeoutReached_internal();
static void stopWebServer_internal(); 
// Web Server Handler functions
static void handleRoot_internal();
static void handleDownload_internal();
static void handleSetTimeForm_internal();
static void handleSetTimeSubmit_internal();
static void handleNotFound_internal();

// --- PUBLIC Web Server Functions (implementations of declarations in web_server.h) ---
bool activateWebServer() {
    // A defensive check: Don't start if the server is already running
    if (isServerRunning) {
        return true;
    }

    // Pre-condition check: ensure Wi-Fi is already on and in AP mode
    if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
        LOG_ERROR(LOG_TAG, "Wi-Fi is not in AP mode. Cannot start server.");
        return false;
    }

    setupWebServerRoutes_internal();
    startWebServer_internal();
    resetWebServerActivityTimer_internal();
    return true;
}

void handleWebServerClients() {
    if (isServerRunning) {
        server.handleClient();
    }
}

bool stopWebServerIfIdle() {
    if (isServerRunning && isWebServerTimeoutReached_internal()) {
        stopWebServer_internal();
        return true;
    }
    return false;
}

bool isWebServerActive() {
    return isServerRunning;
}

// --- PRIVATE functions ---
// Helper functions
static void startWebServer_internal() {
    server.begin();
    isServerRunning = true;
    webServerLastActivityTime = millis(); // Initialize activity time after server is up and running
    LOG_INFO(LOG_TAG, "HTTP server started.");
    LOG_INFO(LOG_TAG, "Connect your phone/device to the '%s' Wi-Fi network.", AP_SSID);
    LOG_INFO(LOG_TAG, "Then open a web browser and go to http://%s/", WiFi.softAPIP().toString().c_str());
    
}

static void resetWebServerActivityTimer_internal() {
    webServerLastActivityTime = millis();
}

static bool isWebServerTimeoutReached_internal() {
    return (millis() - webServerLastActivityTime >= WEB_SERVER_INACTIVITY_TIMEOUT);
}

static void stopWebServer_internal() {
    if (isServerRunning) {
        server.stop();
        isServerRunning = false;
        LOG_INFO(LOG_TAG, "Server stopped.");
    }
}

// Web Server Handler Implementations functions
static void setupWebServerRoutes_internal() {
    server.on("/", handleRoot_internal);
    server.on("/download_data", handleDownload_internal);
    server.on("/settings", handleSetTimeForm_internal);
    server.on("/set_time_submit", handleSetTimeSubmit_internal);
    server.onNotFound(handleNotFound_internal);
    // Add more routes here as needed
}

static void handleRoot_internal() {
    resetWebServerActivityTimer_internal(); // Reset timer on activity

    float h = DataLogger_getLastHumidity();
    float t = DataLogger_getLastTemperature();

    LOG_DEBUG(LOG_TAG, "Handling root request, current humidity: %.1f, temperature: %.1f", h, t);

    String html = "<h1>ESP32 Temp & Humidity Logger</h1>";
    html += "<p><strong>Current Reading:</strong></p>";
    if (isnan(h) || isnan(t)) {
        html += "<p>Failed to read from DHT sensor!</p>";
    } else {
        html += "<p>Humidity: " + String(h, 1) + " %</p>";
        html += "<p>Temperature: " + String(t, 1) + " &deg;C</p>";
    }

    html += "<p>Current ESP32 Time: <strong>" + getFormattedTime() + "</strong></p>"; // From getFormattedTime() global
    html += "<p><a href=\"/download_data\">Download Historical Data (CSV)</a></p>";
    html += "<p><a href=\"/settings\">Set Date & Time</a></p>";
    html += "<p><em>Data logged every " + String(LOG_INTERVAL_SECONDS) + " seconds.</em></p>"; // From LOG_INTERVAL_SECONDS global
    // html = "<meta http-equiv='refresh' content='5'>" + html; // Refresh every 5 seconds

    server.send(200, "text/html", html);
}

static void handleDownload_internal() {
    resetWebServerActivityTimer_internal(); // Reset timer on activity

    if (!LittleFS.begin()) {
        server.send(500, "text/plain", "LittleFS not mounted!");
        return;
    }

    File dataFile = LittleFS.open(LOG_FILE_NAME, "r"); // Using global logFileName
    if (!dataFile) {
        server.send(404, "text/plain", "Data file not found on LittleFS!");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"datalog.csv\"");
    server.sendHeader("Content-type", "text/csv");
    server.sendHeader("Connection", "close");

    const size_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    size_t bytesRead = 0;

    LOG_INFO(LOG_TAG, "Serving %s...", LOG_FILE_NAME);
    while (dataFile.available()) {
        bytesRead = dataFile.read(buffer, bufferSize);
        server.client().write(buffer, bytesRead);
        yield();
    }
    LOG_INFO(LOG_TAG, "File transfer complete.");

    dataFile.close();
}

static void handleSetTimeForm_internal() {
    resetWebServerActivityTimer_internal(); // Reset timer on activity

    String html = "<h1>Set ESP32 Date & Time</h1>";
    html += "<form action=\"/set_time_submit\" method=\"get\">";
    html += "<label for=\"year\">Year:</label> <input type=\"number\" id=\"year\" name=\"year\" value=\"2025\" min=\"2000\" max=\"2100\"><br>";
    html += "<label for=\"month\">Month:</label> <input type=\"number\" id=\"month\" name=\"month\" value=\"7\" min=\"1\" max=\"12\"><br>";
    html += "<label for=\"day\">Day:</label> <input type=\"number\" id=\"day\" name=\"day\" value=\"5\" min=\"1\" max=\"31\"><br>";
    html += "<label for=\"hour\">Hour:</label> <input type=\"number\" id=\"hour\" name=\"hour\" value=\"12\" min=\"0\" max=\"23\"><br>";
    html += "<label for=\"minute\">Minute:</label> <input type=\"number\" id=\"minute\" name=\"minute\" value=\"0\" min=\"0\" max=\"59\"><br>";
    html += "<label for=\"second\">Second:</label> <input type=\"number\" id=\"second\" name=\"second\" value=\"0\" min=\"0\" max=\"59\"><br>";
    html += "<br><input type=\"submit\" value=\"Set Time\">";
    html += "</form>";
    html += "<p>Current ESP32 Time: <strong>" + getFormattedTime() + "</strong></p>";
    html += "<p><a href=\"/\">Back to Home</a></p>";
    server.send(200, "text/html", html);
}

static void handleSetTimeSubmit_internal() {
    resetWebServerActivityTimer_internal(); // Reset timer on activity

    int year = server.arg("year").toInt();
    int month = server.arg("month").toInt();
    int day = server.arg("day").toInt();
    int hour = server.arg("hour").toInt();
    int minute = server.arg("minute").toInt();
    int second = server.arg("second").toInt();

    struct tm timeinfo;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = -1;

    time_t newTime = mktime(&timeinfo);

    if (newTime == (time_t)-1) {
        String html = "<h1>Error Setting Time</h1><p>Invalid date/time provided. Please check values.</p>";
        html += "<p><a href=\"/settings\">Back to Settings</a></p>";
        server.send(200, "text/html", html);
        LOG_WARN(LOG_TAG, "Invalid time provided by user.");
        return;
    }

    struct timeval tv;
    tv.tv_sec = newTime;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    LOG_INFO(LOG_TAG, "Time set to: %s", getFormattedTime().c_str());

    String html = "<h1>Time Set Successfully!</h1>";
    html += "<p>New ESP32 Time: <strong>" + getFormattedTime() + "</strong></p>";
    html += "<p>Remember, this time will be lost on power cycle without an external RTC module.</p>";
    html += "<p><a href=\"/\">Back to Home</a></p>";
    server.send(200, "text/html", html);
}

static void handleNotFound_internal() {
    resetWebServerActivityTimer_internal(); // Reset timer on activity

    String message = "404 Not Found\n\n";
    message += "URI: ";
    message += server.uri(); // Get the URI that was requested
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args(); // Number of arguments
    for (uint8_t i = 0; i < server.args(); i++) {
        message += "\n " + server.argName(i) + ": " + server.arg(i);
    }
    LOG_WARN(LOG_TAG, "Unhandled request for URI: %s", server.uri().c_str());
    server.send(404, "text/plain", message);
}
