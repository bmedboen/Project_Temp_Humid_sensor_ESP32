🌍 ESP32 Ultra-Low-Power Environment Logger
A production-grade, state-driven environmental data logger built for the ESP32 ecosystem. Designed from the ground up for maximum battery life, this system utilizes a hierarchical time-sync architecture, dual-layer flash storage (LittleFS), and an asynchronous web interface with robust network fallback mechanisms.

Supports both standard ESP32 (Xtensa) and modern ESP32-C6 (RISC-V) architectures via dynamic PlatformIO build flags.

✨ Enterprise-Grade Features
⚡ Ultra-Low Power State Machine: The core logic is decoupled into a strict state machine (app_controller). The CPU is only awake for the absolute minimum time required to read sensors, write to flash, and calculate the exact microsecond overhead for the next deep sleep cycle.

💾 Dual-Layer LittleFS Storage:

User Data (/datalog.csv): Non-volatile storage of Temperature and Humidity data. Downloadable via the Web UI.

System Diagnostics (/system.log): An internal "Flight Recorder" tracking system states, boot reasons, and network events. Utilizes a C++ Zero-Cost Abstraction macro system to completely compile-out debug logs in production builds, saving flash memory. Auto-rotates at 20KB.

⏱️ Hierarchical Time Synchronization: Guarantees precise timestamps without constant WiFi overhead:

Primary: External DS3231 RTC (I2C) - Temperature Compensated (TCXO) for hardware accuracy.

Secondary: Internal ESP32 RTC - Maintains time during deep sleep if the external module fails.

Fallback: NTP via WiFi - Only activated if both hardware clocks lose power. Automatically calibrates the External RTC upon successful sync.

🛜 Smart Network & Asynchronous Web UI:

Dual-Mode Connectivity: Tries to connect to a known Station (STA) network. If unreachable, gracefully falls back to Access Point (AP) mode.

mDNS Support: Accessible locally via http://esp32logger-[mac].local.

Async Web Server: Non-blocking web interface to view live data, download CSV logs, update system time, and modify NVS-stored WiFi credentials safely.

📱 On-Demand Local UI: SSD1306 OLED display only turns on during user interaction (hardware interrupt wakeup), showing live environment data and dynamic network assignment status.

🧠 Software Architecture
The application flow is strictly governed by app_controller.cpp acting as the conductor, moving through predefined states to ensure memory and power safety:

STATE_INIT: Mounts LittleFS, initializes NVS, and executes the hierarchical time sync.

STATE_CHECK_WAKEUP: Interrogates the ESP32 wakeup reason (Timer vs. GPIO Button). Decides if the system should stay awake for the Interactive UI or go back to sleep immediately.

STATE_LOGGING: Reads DHT11/22 sensors, formats the timestamp, and performs a direct-append write to /datalog.csv. Updates RTC memory (RTC_DATA_ATTR) with the latest values.

STATE_INTERACTIVE: (Conditional) Spins up I2C for the OLED, starts the Dual-Mode WiFi, and launches the Async Web Server. Yields CPU and monitors an inactivity timeout.

STATE_PREPARE_SLEEP: Calculates the precise deep sleep duration (accounting for boot and execution overhead) and configures the hardware (EXT0 for Xtensa, GPIO for RISC-V) before shutting down the SoC.

🛠️ Hardware Requirements
Microcontroller: ESP32 DevKit V1 (Classic) OR DFRobot FireBeetle 2 ESP32-C6.

Sensor: DHT11 or DHT22 Temperature & Humidity Sensor.

Display: 0.96" I2C OLED (SSD1306).

RTC Module: DS3231 I2C Real-Time Clock.

Power: 18650 Li-Ion Battery (or equivalent) recommended for deployment.

🚀 Getting Started
1. Installation & Dependencies
This project uses . All library dependencies (Adafruit GFX, RTClib, ESPAsyncWebServer, etc.) are automatically managed via platformio.ini.

2. Configuration (secrets.h)
Create a local file src/secrets.h to define your factory default WiFi credentials. This file is excluded from version control for security.

3. Build Environments
Select the appropriate environment in PlatformIO based on your hardware. This automatically injects the correct pinouts and sleep mechanics:

env:firebeetle_c6_dht11 (Uses pioarduino core for RISC-V support)

env:esp32_classic_dht11 (Uses standard Espressif core)

4. Flash & Monitor
Upload the code and monitor the output. Set -D CORE_DEBUG_LEVEL=4 in platformio.ini to see the full diagnostic flow in the terminal.

⚙️ Administration & Usage
Headless Mode (Default): The device spends 99.9% of its life in Deep Sleep. It wakes up at the interval defined by LOG_INTERVAL_SECONDS, logs data, and sleeps.

Accessing the UI: Press the physical Wakeup Button. The OLED will turn on. Connect to the displayed IP or mDNS address in your browser.

Updating WiFi: If you move the logger to a new location, wait for it to spin up its Access Point (ESP32_DataLogger). Connect your phone to this AP, navigate to http://192.168.4.1, and update the credentials via the Web UI. They will be saved permanently to the NVS partition.

🗺️ Future Roadmap (TO-DO)
Interactive UI Extensions: Reset the web server inactivity timer upon subsequent physical button presses. Cycle OLED display views via button interrupts.

Scale & Storage: SD Card SPI integration for infinite log capacity.

Connectivity: Implement BLE provisioning and Over-The-Air (OTA) firmware updates.

Dashboarding: Split web assets into separate files and build a persistent cloud dashboard (MQTT / ESPHome integration).