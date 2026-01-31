// system_logger.cpp

#include "system_logger.h"
#include <stdarg.h>
#include <Arduino.h>

RTC_DATA_ATTR char rtcLogBuffer[LOG_BUFFER_SIZE];
RTC_DATA_ATTR int bufferHead = 0; 
RTC_DATA_ATTR bool bufferWrapped = false;

// --- Helper: Print raw bytes safely in chunks ---
void printRawSafe(const char* data, int length) {
    int written = 0;
    while (written < length) {
        // Send in chunks of 128 bytes to prevent hardware buffer overflow
        int chunk = length - written;
        if (chunk > 128) chunk = 128;
        
        Serial.write((const uint8_t*)&data[written], chunk);
        Serial.flush(); // Wait for hardware to send
        
        written += chunk;
    }
}

void Logger_Init() {
    // Test output for diagnostics
    if (Serial) {
        Serial.printf("\n[DIAGNOSE] BufferHead ved oppstart: %d (Wrapped: %d)\n", bufferHead, bufferWrapped);
    }

    // Basic safety check
    if (bufferHead < 0 || bufferHead >= LOG_BUFFER_SIZE) {
        bufferHead = 0;
        bufferWrapped = false;
        // Do NOT clear bufferWrapped here, or we lose history on deep sleep wake!
    }
}

void writeToRTC(const char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        // Write byte directly to current position
        rtcLogBuffer[bufferHead] = str[i];
        bufferHead++;
        
        // Handle Wrap Around
        if (bufferHead >= LOG_BUFFER_SIZE) {
            bufferHead = 0;
            bufferWrapped = true;
        }
    }
}

void Logger_Log(LogLevel level, const char* tag, const char* format, ...) {
    unsigned long now = millis();
    
    // 1. Prepare colors
    const char* lvlStr = "";
    const char* colorCode = ""; 
    
    switch (level) {
        case LogLevel::Error: lvlStr = "ERR"; colorCode = "\033[31m"; break; 
        case LogLevel::Warn:  lvlStr = "WRN"; colorCode = "\033[33m"; break; 
        case LogLevel::Info:  lvlStr = "INF"; colorCode = "\033[32m"; break; 
        case LogLevel::Debug: lvlStr = "DBG"; colorCode = "\033[36m"; break; 
        default: break;
    }

    // 2. Format message
    char msgBuffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);

    // 3. Write to RTC (Plain text)
    // We add a newline explicitly to ensure structure
    char logLine[320];
    snprintf(logLine, sizeof(logLine), "[%lu] %s [%s] %s\n", now, lvlStr, tag, msgBuffer);
    writeToRTC(logLine);

    // 4. Print to Serial (Live view with colors)
    if (Serial) {
        Serial.printf("[%lu] %s%s [%s] \033[0m%s\n", now, colorCode, lvlStr, tag, msgBuffer);
    }
}

void Logger_FlushRTCtoSerial() {
    if (!Serial) return;

    // 1. WARMUP: Send an empty line and wait.
    // This wakes up the terminal before we send important text.
    Serial.println();       
    Serial.flush();
    delay(100);             

    // 2. HEADER: Print header and force a newline afterwards.
    Serial.println(">>> RTC FLIGHT RECORDER HISTORY START >>>");
    Serial.flush();

    // 3. PART 1: OLD HISTORY (The Tail)
    // If the buffer has wrapped around, we must first print the end of the buffer.
    if (bufferWrapped) {
        int scanIdx = bufferHead;
        bool foundSync = false;
        
        // Scan for the first 'newline' to find a clean starting point
        while (scanIdx < LOG_BUFFER_SIZE) {
            if (rtcLogBuffer[scanIdx] == '\n') {
                foundSync = true;
                scanIdx++; // Start immediately AFTER the newline
                break;
            }
            scanIdx++;
        }

        if (foundSync && scanIdx < LOG_BUFFER_SIZE) {
            int length = LOG_BUFFER_SIZE - scanIdx;
            printRawSafe(&rtcLogBuffer[scanIdx], length);
            
            // --- CRITICAL FIX ---
            // Force a newline here, in case the old history ended 
            // in the middle of a sentence (like 'lay turned off').
            Serial.println(); 
        }
    }
    
    // 4. PART 2: NEW HISTORY (The Head)
    // Print from the start of the buffer up to the current write position.
    if (bufferHead > 0) {
        printRawSafe(rtcLogBuffer, bufferHead);
        
        // --- CRITICAL FIX ---
        // Force a newline after the last log line as well.
        Serial.println(); 
    }
    
    // 5. FOOTER: Finish cleanly
    Serial.println("<<< RTC FLIGHT RECORDER HISTORY END <<<");
    
    // 6. RESET AND BREATHE
    // Give the terminal a 50ms pause before the next "live" log arrives 
    // from the main loop. This prevents sticking artifacts.
    Serial.print("\033[0m"); 
    Serial.flush(); 
    delay(50); 
}
