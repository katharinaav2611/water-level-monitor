#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// HC-SR04 ULTRASONIC SENSOR CONFIGURATION
// ==========================================

// GPIO Pin Definitions
#define TRIG_PIN 25        // Trigger pin for HC-SR04
#define ECHO_PIN 26        // Echo pin for HC-SR04

// Tank Dimensions (in centimeters)
#define MAX_TANK_DEPTH 100 // Maximum water depth in your tank
#define MIN_TANK_DEPTH 5   // Minimum sensor distance from bottom

// Sensor Accuracy
#define SENSOR_SAMPLES 10  // Number of readings to average (reduces noise)
#define SAMPLE_DELAY 50    // Delay between samples (ms)

// ==========================================
// TELEMETRY SETTINGS
// ==========================================

#define TELEMETRY_INTERVAL 30000  // Send data every 30 seconds
#define SENSOR_READ_TIMEOUT 30000  // Sensor read timeout (ms)

// ==========================================
// WIFI CONFIGURATION
// ==========================================

// WiFi Credentials (configured in secrets.h)
// #define WIFI_SSID "your_ssid"
// #define WIFI_PASSWORD "your_password"

#define WIFI_TIMEOUT 20000          // WiFi connection timeout (ms)
#define WIFI_RECONNECT_INTERVAL 60000 // Reconnect attempt interval (ms)

// ==========================================
// AZURE IOT HUB CONFIGURATION
// ==========================================

// IoT Hub Connection (configured in secrets.h)
// #define IOT_HUB_NAME "your-iot-hub-name"
// #define DEVICE_ID "esp32-water-monitor"
// #define DEVICE_KEY "your_device_primary_key"

#define IOT_HUB_SUFFIX ".azure-devices.net"
#define MQTT_PORT 8883                    // TLS/SSL port for MQTT
#define IOT_HUB_CONNECT_TIMEOUT 10000     // IoT Hub connection timeout (ms)
#define IOT_HUB_RECONNECT_INTERVAL 30000  // Reconnect attempt interval (ms)

// ==========================================
// LOGGING AND DEBUG
// ==========================================

#define SERIAL_BAUD 115200  // Serial monitor baud rate
#define DEBUG_MODE true     // Enable debug logging (true/false)

#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// ==========================================
// TELEMETRY DATA STRUCTURE
// ==========================================

// JSON telemetry payload structure:
// {
//   "deviceId": "esp32-water-monitor",
//   "timestamp": "2026-05-11T10:30:45Z",
//   "depth_cm": 45.2,
//   "water_level_percent": 45,
//   "temperature": 28.5,
//   "humidity": 65,
//   "signal_strength": -65
// }

// ==========================================
// PERFORMANCE TUNING
// ==========================================

#define ENABLE_DEEP_SLEEP false       // Enable deep sleep between readings
#define DEEP_SLEEP_DURATION 60000000  // Deep sleep duration (microseconds)

#endif // CONFIG_H
