// web_server.cpp

#include "web_server.h" // Include its own header file

#include <sys/time.h>   // For settimeofday
#include <FS.h> 
#include <LittleFS.h> 

#include "config.h"
#include "data_logger.h" 
#include "time_manager.h" // For getFormattedTime()

// --- Global variables (definitions from web_server.h) ---
WebServer server(WEBSERVER_PORT); // Define the WebServer object (port 80)
unsigned long webServerLastActivityTime = 0;

// --- PRIVATE Web Server Handler Functions ---
// These functions are only called internally by the web_server module
void handleRoot_internal();
void handleDownload_internal();
void handleSetTimeForm_internal();
void handleSetTimeSubmit_internal();
void handleNotFound_internal();

// --- PUBLIC Web Server Functions (implementations of declarations in web_server.h) ---
void initWebServerAP() {
    Serial.print("Web Server: Setting up AP ");
    Serial.print(AP_SSID);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    delay(100); // Give WiFi time to start
    Serial.println(" READY");
    Serial.print("Web Server: AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    webServerLastActivityTime = millis(); // Initialize activity time after Wi-Fi is up
}

void setupWebServerRoutes() {
    server.on("/", handleRoot_internal);
    server.on("/download_data", handleDownload_internal);
    server.on("/settings", handleSetTimeForm_internal);
    server.on("/set_time_submit", handleSetTimeSubmit_internal);
    server.onNotFound(handleNotFound_internal);
    // Add more routes here as needed
}

void startWebServer() {
    server.begin();
    Serial.println("Web Server: HTTP server started.");
    Serial.println("Web Server: Connect your phone/device to the '" + String(AP_SSID) + "' Wi-Fi network.");
    Serial.println("Web Server: Then open a web browser and go to http://" + WiFi.softAPIP().toString() + "/");
}

void handleWebServerClients() {
    server.handleClient();
}

bool isWebServerActive() {
    return (WiFi.getMode() == WIFI_AP);
}

void resetWebServerActivityTimer() {
    webServerLastActivityTime = millis();
}

bool isWebServerTimeoutReached() {
    return (millis() - webServerLastActivityTime >= WEB_SERVER_INACTIVITY_TIMEOUT);
}

// --- PRIVATE Web Server Handler Implementations ---

void handleRoot_internal() {
    resetWebServerActivityTimer(); // Reset timer on activity

    float h = DataLogger_getLastHumidity();
    float t = DataLogger_getLastTemperature();

    Serial.println("Web Server: Handling root request, current humidity: " + String(h) + ", temperature: " + String(t));

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

void handleDownload_internal() {
    resetWebServerActivityTimer(); // Reset timer on activity

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

    Serial.println("Web Server: Serving " + String(LOG_FILE_NAME) + "...");
    while (dataFile.available()) {
        bytesRead = dataFile.read(buffer, bufferSize);
        server.client().write(buffer, bytesRead);
        yield();
    }
    Serial.println("Web Server: File transfer complete.");

    dataFile.close();
}

void handleSetTimeForm_internal() {
    resetWebServerActivityTimer(); // Reset timer on activity

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

void handleSetTimeSubmit_internal() {
    resetWebServerActivityTimer(); // Reset timer on activity

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
        Serial.println("Web Server: Invalid time provided by user.");
        return;
    }

    struct timeval tv;
    tv.tv_sec = newTime;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    Serial.print("Web server: Time set to: ");
    Serial.println(getFormattedTime());

    String html = "<h1>Time Set Successfully!</h1>";
    html += "<p>New ESP32 Time: <strong>" + getFormattedTime() + "</strong></p>";
    html += "<p>Remember, this time will be lost on power cycle without an external RTC module.</p>";
    html += "<p><a href=\"/\">Back to Home</a></p>";
    server.send(200, "text/html", html);
}

void handleNotFound_internal() {
    resetWebServerActivityTimer(); // Reset timer on activity

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
    Serial.println("Web Server: Unhandled request for URI: " + server.uri()); // Log to serial too
    server.send(404, "text/plain", message);
}
