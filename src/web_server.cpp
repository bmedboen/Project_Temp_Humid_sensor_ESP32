// web_server.cpp

#include "web_server.h"

#include <WiFi.h>
#include <AsyncTCP.h>           // Async TCP Library
#include <ESPAsyncWebServer.h>  // Async Web Server Library
#include <sys/time.h>           // For settimeofday
#include <FS.h> 
#include <LittleFS.h> 

#include "config.h"
#include "data_logger.h" 
#include "time_manager.h" 
#include "settings_manager.h" 
#include "system_logger.h" 
#include "external_rtc.h"

#define LOG_TAG "WEB"

// --- Global Variables ---
// Create AsyncWebServer object on port 80
AsyncWebServer server(WEBSERVER_PORT); 

static uint64_t webServerLastActivityTime = 0;
static bool isServerRunning = false;

// --- Helper Functions Declaration ---
static void setupWebServerRoutes_internal();
static void resetWebServerActivityTimer_internal();
static bool isWebServerTimeoutReached_internal();
static void stopWebServer_internal();
String getRootHtml(); // Helper to generate HTML

// --- PUBLIC FUNCTIONS ---

bool activateWebServer() {
    if (isServerRunning) {
        return true;
    }

    // Safety Check: WiFi radio must be ON (AP, STA, or AP_STA)
    if (WiFi.getMode() == WIFI_OFF) {
        LOG_ERROR(LOG_TAG, "Wi-Fi is OFF. Cannot start server.");
        return false;
    }

    // Configure URL routes and handlers
    setupWebServerRoutes_internal();
    
    // Start the server
    server.begin(); 
    isServerRunning = true;
    resetWebServerActivityTimer_internal();
    
    LOG_INFO(LOG_TAG, "Async HTTP server started.");
    
    // Log access URLs based on current mode
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        LOG_INFO(LOG_TAG, "Access via AP: http://%s/", WiFi.softAPIP().toString().c_str());
    }
    if (WiFi.status() == WL_CONNECTED) {
        LOG_INFO(LOG_TAG, "Access via LAN: http://%s/", WiFi.localIP().toString().c_str());
    }

    return true;
}

void handleWebServerClients() {
    // NOT NEEDED for AsyncWebServer. 
    // Kept empty to maintain compatibility with app_controller structure.
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

// --- INTERNAL HELPER FUNCTIONS ---

static void resetWebServerActivityTimer_internal() {
    webServerLastActivityTime = millis();
}

static bool isWebServerTimeoutReached_internal() {
    return (millis() - webServerLastActivityTime >= WEB_SERVER_INACTIVITY_TIMEOUT);
}

static void stopWebServer_internal() {
    if (isServerRunning) {
        server.end(); // Use .end() for AsyncWebServer
        delay(100); // Allow time for server to close connections
        isServerRunning = false;
        LOG_INFO(LOG_TAG, "Server stopped due to inactivity.");
    }
}

// --- HTML GENERATOR ---
String getRootHtml() {
    float h = DataLogger_getLastHumidity();
    float t = DataLogger_getLastTemperature();
    String timeStr = DataLogger_getLastLogTime();

    String html = R"raw(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Logger</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; text-align: center; margin: 20px; background-color: #f4f7f6; color: #333; }
        .container { max-width: 800px; margin: 0 auto; }
        
        /* Card Style */
        .card { background: #fff; padding: 20px; margin-bottom: 20px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        
        /* Typography */
        h2 { color: #2c3e50; margin-bottom: 10px; font-weight: 600;}
        h3 { border-bottom: 2px solid #f0f0f0; padding-bottom: 10px; margin-top: 0; color: #555; font-size: 1.2rem; }
        
        /* Sensor Values */
        .sensor-row { display: flex; justify-content: space-around; align-items: center; margin-bottom: 10px; }
        .val { font-size: 2.2em; font-weight: bold; color: #3498db; }
        .unit { font-size: 0.5em; color: #7f8c8d; font-weight: normal; margin-left: 2px; }
        .timestamp-label { font-size: 0.85rem; color: #888; margin-top: 5px; font-style: italic; }

        /* Inputs & Buttons */
        button, input[type=submit] { background: #3498db; color: white; border: none; padding: 12px; border-radius: 8px; cursor: pointer; width: 100%; font-size: 1rem; margin-top: 15px; font-weight: 500; transition: background 0.2s;}
        button:hover, input[type=submit]:hover { background: #2980b9; }
        input { padding: 12px; box-sizing: border-box; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 6px; font-size: 1rem; }
        
        /* Form Layout Helpers */
        .form-row { display: flex; gap: 10px; flex-wrap: wrap; }
        .form-row input { flex: 1; min-width: 70px; } /* Ensures inputs assume full width on very small screens or share space on large ones */

        /* Data Table Styles */
        .table-scroll { max-height: 400px; overflow-y: auto; display: block; border: 1px solid #eee; border-radius: 4px; }
        table { width: 100%; border-collapse: collapse; font-size: 0.85rem; }
        th, td { padding: 12px 8px; text-align: center; border-bottom: 1px solid #eee; }
        th { background-color: #f8f9fa; color: #555; font-weight: 600; position: sticky; top: 0; box-shadow: 0 1px 2px rgba(0,0,0,0.05); }
        tr:nth-child(even) { background-color: #fbfbfb; }
        .loading { padding: 20px; color: #888; font-style: italic; }
    </style>
</head>
<body>
    <div class="container">
        <h2>ESP32 Environment Logger</h2>
        
        <div class="card">
            <h3>Current Readings</h3>
            <div class="sensor-row">
                <div>
                    <div style="font-size:0.9rem; color:#888;">Temperature</div>
                    <div class="val">)raw";
    html += (isnan(t) ? "--" : String(t, 1));
    html += R"raw(<span class="unit">&deg;C</span></div>
                </div>
                <div>
                    <div style="font-size:0.9rem; color:#888;">Humidity</div>
                    <div class="val">)raw";
    html += (isnan(h) ? "--" : String(h, 1));
    html += R"raw(<span class="unit">%</span></div>
                </div>
            </div>
            <div class="timestamp-label">Measured at: )raw";
    html += timeStr;
    html += R"raw(</div>
        </div>

        <div class="card">
            <h3>Log History</h3>
            <div class="table-scroll">
                <div id="dataTable" class="loading">Fetching data...</div>
            </div>
            <button onclick="location.href='/download'">Download CSV File</button>
        </div>

        <div class="card">
            <h3>System Settings</h3>
            
            <form action="/set_time" method="GET">
                <div style="text-align:left; margin-bottom:5px; font-weight:bold; color:#666;">Set Date & Time</div>
                
                <div class="form-row">
                   <input type="number" name="Y" placeholder="Year" required style="flex:2">
                   <input type="number" name="M" placeholder="Month" required>
                   <input type="number" name="D" placeholder="Day" required>
                </div>

                <div class="form-row">
                   <input type="number" name="h" placeholder="Hour" required>
                   <input type="number" name="m" placeholder="Minute" required>
                </div>
                
                <input type="submit" value="Update Time" style="background: #7f8c8d;"> 
                
                <div style="font-size: 0.8rem; color: #999; margin-top: 5px;">Current System Time: )raw";
    html += timeStr;
    html += R"raw(</div>
            </form>
            
            <hr style="border:0; border-top:1px solid #eee; margin:25px 0;">

            <form action="/set_wifi" method="POST">
                <div style="text-align:left; margin-bottom:5px; font-weight:bold; color:#666;">WiFi Configuration</div>
                <input type="text" name="ssid" placeholder="Network Name (SSID)" required>
                <input type="password" name="pass" placeholder="Network Password">
                <input type="submit" value="Save Credentials & Restart" style="background:#e74c3c">
            </form>
            <p><small style="color:#999;">Active SSID: )raw";
    html += Settings.getWifiSSID();
    html += R"raw(</small></p>
        </div>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', function() {
            fetch('/download')
            .then(response => {
                if (!response.ok) throw new Error("No data");
                return response.text();
            })
            .then(csvText => {
                const lines = csvText.trim().split('\n');
                if (lines.length < 2) {
                    document.getElementById('dataTable').innerHTML = "<div style='padding:20px'>No data logged yet.</div>";
                    return;
                }

                const headers = lines[0].split(',');
                // Reverse to show newest first
                const dataLines = lines.slice(1).reverse();

                let tableHtml = '<table><thead><tr>';
                headers.forEach(h => tableHtml += '<th>' + h + '</th>');
                tableHtml += '</tr></thead><tbody>';

                dataLines.forEach(line => {
                    const cols = line.split(',');
                    if(cols.length > 1) { 
                        tableHtml += '<tr>';
                        cols.forEach(col => tableHtml += '<td>' + col + '</td>');
                        tableHtml += '</tr>';
                    }
                });
                tableHtml += '</tbody></table>';

                document.getElementById('dataTable').innerHTML = tableHtml;
            })
            .catch(error => {
                console.error('Error:', error);
                document.getElementById('dataTable').innerHTML = "<div style='padding:20px; color:red'>Error loading data.</div>";
            });
        });
    </script>
</body>
</html>
)raw";
    return html;
}

// --- SETUP ROUTES (LAMBDA FUNCTIONS) ---
static void setupWebServerRoutes_internal() {

    // 1. ROOT (Home Page)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        resetWebServerActivityTimer_internal();
        request->send(200, "text/html", getRootHtml());
    });

    // 2. DOWNLOAD (Streams file from LittleFS - Non-blocking)
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        resetWebServerActivityTimer_internal();
        if (LittleFS.exists(LOG_FILE_NAME)) {
            // send(FileSystem, Path, MimeType, Download=true)
            request->send(LittleFS, LOG_FILE_NAME, "text/csv", true); 
        } else {
            request->send(404, "text/plain", "Log file not found.");
        }
    });

    // 3. SET TIME (GET Request)
    server.on("/set_time", HTTP_GET, [](AsyncWebServerRequest *request){
        resetWebServerActivityTimer_internal();
        
        // Validate inputs
        if (request->hasParam("Y") && request->hasParam("M") && request->hasParam("D") && request->hasParam("h") && request->hasParam("m")) {
            struct tm t;
            t.tm_year = request->getParam("Y")->value().toInt() - 1900;
            t.tm_mon  = request->getParam("M")->value().toInt() - 1;
            t.tm_mday = request->getParam("D")->value().toInt();
            t.tm_hour = request->getParam("h")->value().toInt();
            t.tm_min  = request->getParam("m")->value().toInt();
            t.tm_sec  = 0;
            t.tm_isdst = -1;
            
            TimeManager_setTime(t, true);
            
            request->send(200, "text/html", "<h1>Time Updated</h1><br><a href='/'>Back to Home</a>");
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    // 4. SET WIFI (POST Request)
    server.on("/set_wifi", HTTP_POST, [](AsyncWebServerRequest *request){
        resetWebServerActivityTimer_internal();
        
        // Check if SSID was posted
        if (request->hasParam("ssid", true)) {
            String s = request->getParam("ssid", true)->value();
            String p = request->hasParam("pass", true) ? request->getParam("pass", true)->value() : "";
            
            // Save to NVS via SettingsManager
            Settings.saveWifiCredentials(s, p);
            
            request->send(200, "text/html", "<h1>Credentials Saved. Restarting...</h1>");
            
            // Allow response to send before restarting
            delay(1000); 
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Missing SSID");
        }
    });

    // 404 NOT FOUND
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "404 Not Found");
    });
}
