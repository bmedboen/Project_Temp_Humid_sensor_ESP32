// sleep_manager.h

#pragma once

#include <Arduino.h>   // For delay(), millis(), etc.

void goToDeepSleep(unsigned long deep_sleep_time_us); // Function to enter deep sleep mode with specified time in microseconds
