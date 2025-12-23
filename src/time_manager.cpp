// time_manager.cpp

#include "time_manager.h"
#include "system_logger.h" // New include for logging

#define LOG_TAG "TIME" // Define a tag for TimeManager module logs

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Not Set";
  }
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}
