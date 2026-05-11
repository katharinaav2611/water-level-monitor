# ESP32 Water Level Monitor Firmware

Arduino firmware for ESP32 DevKitV1 with HC-SR04 ultrasonic sensor.

## Prerequisites

1. **Arduino IDE** (version 1.8.0 or higher)
2. **ESP32 Board Package** - Install via Arduino IDE:
   - File → Preferences → Additional Boards Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board Manager → Search "esp32" → Install

3. **Required Libraries** - Install via Arduino IDE (Sketch → Include Library → Manage Libraries):
   - `PubSubClient` by Nick O'Leary (MQTT client)
   - `Azure IoT Hub` by Microsoft (optional, or use MQTT directly)
   - `AzureIoTUtility` by Microsoft (optional)

## Hardware Connections

### HC-SR04 Ultrasonic Sensor
```
HC-SR04 Pin    ESP32 GPIO Pin
─────────────────────────────
VCC      →     5V (or 3.3V with voltage divider)
GND      →     GND
TRIG     →     GPIO 25 (D25)
ECHO     →     GPIO 26 (D26)
```

### USB Connection
- Connect ESP32 to your computer via USB-C for programming

## Configuration

### 1. Copy Secrets Template
```bash
cp secrets_example.h secrets.h
```

### 2. Edit `secrets.h`
```cpp
// WiFi Credentials
const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";

// Azure IoT Hub
const char* IOT_HUB_NAME = "your-iot-hub-name";
const char* DEVICE_ID = "esp32-water-monitor";
const char* DEVICE_KEY = "your_device_primary_key";
```

### 3. Edit `config.h` (Optional)
```cpp
// HC-SR04 Pins
#define TRIG_PIN 25
#define ECHO_PIN 26

// Tank Configuration (in cm)
#define MAX_TANK_DEPTH 100  // Maximum water depth in your tank
#define MIN_TANK_DEPTH 5    // Minimum sensor distance

// Telemetry Settings
#define TELEMETRY_INTERVAL 30000  // Send data every 30 seconds
#define SENSOR_SAMPLES 10          // Number of readings to average
```

## Upload to ESP32

1. **Connect ESP32** to your computer via USB
2. **Select Board and Port:**
   - Tools → Board → ESP32 Dev Module
   - Tools → Port → COM3 (or your ESP32 port)
3. **Upload Sketch:**
   - Sketch → Upload (or Ctrl+U)
   - Wait for upload to complete ("Done uploading" message)

## Monitoring

View serial output to verify operation:

1. **Open Serial Monitor:**
   - Tools → Serial Monitor (or Ctrl+Shift+M)
   - Set Baud Rate: **115200**

2. **Expected Output:**
```
Initializing Water Level Monitor...
Connecting to WiFi: MyNetwork
WiFi connected! IP: 192.168.1.100
Connecting to IoT Hub...
IoT Hub connected!
Water Depth: 45.2 cm → 45% full
Telemetry sent to Azure!
```

## Troubleshooting

### Upload Fails
- Check USB cable connection
- Verify COM port in Tools → Port
- Try holding BOOT button while uploading

### WiFi Connection Fails
- Verify SSID and password in `secrets.h`
- Check 2.4GHz WiFi (5GHz not supported on ESP32)
- Ensure ESP32 is in range of router

### No Sensor Readings
- Verify HC-SR04 wiring (TRIG and ECHO pins)
- Check if sensor has power (red LED on module should light)
- Test sensor independently before full integration

### IoT Hub Connection Fails
- Verify device connection string in `secrets.h`
- Check internet connectivity
- Review Azure IoT Hub connection string format
- Ensure device is registered in IoT Hub

## Code Structure

```cpp
void setup()                    // Initialization
void loop()                     // Main loop
float readDistance()            // Read HC-SR04 sensor
float calculateWaterLevel()     // Convert distance to percentage
void sendTelemetry()           // Send data to Azure IoT Hub
void connectWiFi()             // WiFi connection logic
void connectIoTHub()           // IoT Hub connection logic
```

## Performance Notes

- **Update Interval:** 30 seconds (configurable)
- **Sensor Accuracy:** ±0.3 cm (HC-SR04 specification)
- **Power Consumption:** ~80 mA during operation
- **WiFi Range:** Typical ~30 meters (depends on router)

## Next Steps

1. Verify firmware is working (check serial monitor)
2. Confirm telemetry in Azure IoT Hub (check device-to-cloud messages)
3. Deploy web dashboard (see `dashboard/README.md`)
4. Set up Azure Table Storage for data persistence

## References

- [ESP32 Arduino Core Documentation](https://github.com/espressif/arduino-esp32/tree/master/cores/esp32)
- [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Ultrasonic/HC-SR04.pdf)
- [PubSubClient Library](https://github.com/knolleary/pubsubclient)
- [Azure IoT Hub MQTT Support](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support)
