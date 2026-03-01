// system_logger.cpp

#include "system_logger.h"
#include <LittleFS.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_TAG "LOGGER"

void Logger_Init() {
    // 1. Mount LittleFS (true = format on first use)
    if (!LittleFS.begin(true)) {
        if (Serial) Serial.println("ERR: LittleFS mount failed! Logging to file disabled.");
        return;
    }
    
    // 2. File Size Management
    if (LittleFS.exists(LOG_FILE_PATH)) {
        File logFile = LittleFS.open(LOG_FILE_PATH, "r");
        size_t size = 0;
        
        if (logFile) {
            size = logFile.size();
            logFile.close();
        }

        // Reset file if it exceeds the maximum allowed size
        if (size > MAX_LOG_FILE_SIZE) {
            LittleFS.remove(LOG_FILE_PATH);
            if (Serial) Serial.println("\033[33m[WARN] Log file too large. Resetting history.\033[0m");
            
            File newFile = LittleFS.open(LOG_FILE_PATH, "w");
            if (newFile) {
                newFile.println("--- LOG RESET (Size Limit Reached) ---");
                newFile.close();
            }
        }
    }
}

void Logger_Log(LogLevel level, const char* tag, const char* format, ...) {
    
    // 1. Calculate Time 
    unsigned long now = millis();
    unsigned long hours = (now / 3600000);
    unsigned long mins  = (now / 60000) % 60;
    unsigned long secs  = (now / 1000) % 60;
    unsigned long ms    = now % 1000;

    // 2. Define color codes and level strings
    const char* colorCode = "\033[0m"; // Default reset
    const char* levelStr = "[UNK]  ";

    switch (level) {
        case LogLevel::Error: 
            colorCode = "\033[31m"; // Red
            levelStr = "ERROR"; 
            break;
        case LogLevel::Warn:  
            colorCode = "\033[33m"; // Yellow
            levelStr = "WARN "; 
            break;
        case LogLevel::Info:  
            colorCode = "\033[32m"; // Green
            levelStr = "INFO "; 
            break;
        case LogLevel::Debug: 
            colorCode = "\033[36m"; // Cyan
            levelStr = "DEBUG"; 
            break;
        case LogLevel::None:
            return;
    }

    // 3. Format the user message
    char msgBuffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);

    // 4. Create a clean string for the FILE (No ANSI color codes!)
    char fileLogLine[350];
    snprintf(fileLogLine, sizeof(fileLogLine), "[%02lu:%02lu:%02lu.%03lu] [%s] [%s] %s", 
             hours, mins, secs, ms, levelStr, tag, msgBuffer);

    // 5. Print to Serial WITH colors
    if (Serial) {
        Serial.printf("[%02lu:%02lu:%02lu.%03lu] %s[%s] [%s] %s\033[0m\n", 
                      hours, mins, secs, ms, colorCode, levelStr, tag, msgBuffer);
    }

    // 6. Write to Flash Memory (LittleFS)
    File logFile = LittleFS.open(LOG_FILE_PATH, "a");
    if (logFile) {
        logFile.println(fileLogLine);
        logFile.close();
    }
}

void Logger_DumpToSerial() {
    if (!LittleFS.exists(LOG_FILE_PATH)) {
        if (Serial) Serial.println("--- No Log File Found ---");
        return;
    }

    File logFile = LittleFS.open(LOG_FILE_PATH, "r");
    if (!logFile) return;

    if (Serial) {
        Serial.println("\n--- READING SYSTEM LOG FROM FLASH ---");
        while (logFile.available()) {
            Serial.write(logFile.read());
        }
        Serial.println("--- END OF LOG ---\n");
        
        // From RTC logic: Ensure terminal colors are reset and buffer is clear
        Serial.print("\033[0m"); 
        Serial.flush(); 
    }
    logFile.close();
}
