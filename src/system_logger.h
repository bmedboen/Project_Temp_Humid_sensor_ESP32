// system_logger.h
#pragma once
#include <Arduino.h>
#include "config.h"

// Preprocessor definitions for conditional compilation (#if)
#define _LOG_LVL_NONE  0
#define _LOG_LVL_ERROR 1
#define _LOG_LVL_WARN  2
#define _LOG_LVL_INFO  3
#define _LOG_LVL_DEBUG 4

// Safety default
#ifndef SYSTEM_LOG_LEVEL
    #define SYSTEM_LOG_LEVEL _LOG_LVL_INFO
#endif

// C++ Enum for Type Safety in function calls
enum class LogLevel {
    None  = _LOG_LVL_NONE,
    Error = _LOG_LVL_ERROR,
    Warn  = _LOG_LVL_WARN,
    Info  = _LOG_LVL_INFO,
    Debug = _LOG_LVL_DEBUG
};

// Function takes the Enum
void Logger_Init();
void Logger_Log(LogLevel level, const char* tag, const char* format, ...);

// Function to dump the buffered logs to Serial
void Logger_FlushRTCtoSerial();

// Macros use the Defines
#if SYSTEM_LOG_LEVEL >= _LOG_LVL_ERROR
    // Note: We cast the enum to pass it to the function
    #define LOG_ERROR(tag, ...) Logger_Log(LogLevel::Error, tag, __VA_ARGS__)
#else
    #define LOG_ERROR(tag, ...) 
#endif

#if SYSTEM_LOG_LEVEL >= _LOG_LVL_WARN
    #define LOG_WARN(tag, ...)  Logger_Log(LogLevel::Warn,  tag, __VA_ARGS__)
#else
    #define LOG_WARN(tag, ...) 
#endif

#if SYSTEM_LOG_LEVEL >= _LOG_LVL_INFO
    #define LOG_INFO(tag, ...)  Logger_Log(LogLevel::Info,  tag, __VA_ARGS__)
#else
    #define LOG_INFO(tag, ...) 
#endif

#if SYSTEM_LOG_LEVEL >= _LOG_LVL_DEBUG
    #define LOG_DEBUG(tag, ...) Logger_Log(LogLevel::Debug, tag, __VA_ARGS__)
#else
    #define LOG_DEBUG(tag, ...) 
#endif
