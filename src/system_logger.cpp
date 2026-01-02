// system_logger.cpp

#include "system_logger.h"
#include <stdarg.h>

#define LOG_TAG "LOGGER"

void Logger_Init() {
    // Initialization code can be added here if needed in the future
}

void Logger_Log(LogLevel level, const char* tag, const char* format, ...) {
    
    unsigned long now = millis();
    unsigned long hours = (now / 3600000);
    unsigned long mins  = (now / 60000) % 60;
    unsigned long secs  = (now / 1000) % 60;
    unsigned long ms    = now % 1000;

    // 1. Define color codes and level strings
    const char* colorCode = ""; 
    const char* levelStr = "";

    switch (level) {
        case LogLevel::Error: 
            colorCode = "\033[31m"; // Red
            levelStr = "[ERROR]"; 
            break;
        case LogLevel::Warn:  
            colorCode = "\033[33m"; // Yellow
            levelStr = "[WARN] "; 
            break;
        case LogLevel::Info:  
            colorCode = "\033[32m"; // Greenn
            levelStr = "[INFO] "; 
            break;
        case LogLevel::Debug: 
            colorCode = "\033[36m"; // Cyan
            levelStr = "[DEBUG]"; 
            break;
        default: 
            break;
    }

    // 2. Format the user message
    char msgBuffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);

    // 3. Print the final log message in one "package"
    // Format: [Time] COLOR [LEVEL] [TAG] User Message
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] %s%s [%s] \033[0m%s\n", 
                  hours, mins, secs, ms, 
                  colorCode, levelStr, tag, 
                  msgBuffer);
}
