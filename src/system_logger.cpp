// system_logger.cpp

#include "system_logger.h"
#include <stdarg.h>

#define LOG_TAG "LOGGER"

void Logger_Init() {

    LOG_INFO(LOG_TAG, "Logger Module Initialized (Level: %d)", SYSTEM_LOG_LEVEL);
}

void Logger_Log(LogLevel level, const char* tag, const char* format, ...) {

    // Always reset colors first - this prevents color bleeding from the previous line affecting the timestamp
    Serial.print("\033[0m"); 

    unsigned long now = millis();
    unsigned long hours = (now / 3600000);
    unsigned long mins  = (now / 60000) % 60;
    unsigned long secs  = (now / 1000) % 60;
    unsigned long ms    = now % 1000;

    Serial.printf("[%02lu:%02lu:%02lu.%03lu] ", hours, mins, secs, ms);

    switch (level) {
        case LogLevel::Error: Serial.print("\033[31m[ERROR] "); break;
        case LogLevel::Warn:  Serial.print("\033[33m[WARN]  "); break;
        case LogLevel::Info:  Serial.print("\033[32m[INFO]  "); break;
        case LogLevel::Debug: Serial.print("\033[36m[DEBUG] "); break;
        default: break;
    }

    // Print the tag and the reset code in separate statements avoids buffer timing issues
    Serial.printf("[%s] ", tag);
    Serial.print("\033[0m"); 

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.println(buffer);
}
