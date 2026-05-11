#include "config.h"
#include "secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <ArduinoJson.h>

// ==========================================
// GLOBAL VARIABLES
// ==========================================

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Timing variables
unsigned long lastTelemetrySend = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastIoTHubAttempt = 0;

// Connection status
bool wifiConnected = false;
bool iotHubConnected = false;

// Water level data
float currentWaterLevelPercent = 0;
float currentDepthCm = 0;

// ==========================================
// SETUP FUNCTION
// ==========================================

void setup() {
  // Initialize Serial for debugging
  Serial.begin(SERIAL_BAUD);
  delay(100);
  
  DEBUG_PRINTLN("\n\n");
  DEBUG_PRINTLN("===============================================");
  DEBUG_PRINTLN("  ESP32 Water Level Monitor - Initializing");
  DEBUG_PRINTLN("===============================================");
  
  // Configure sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  DEBUG_PRINTLN("[INIT] Sensor pins configured");
  
  // Set time from NTP server (required for TLS/SSL)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  DEBUG_PRINTLN("[INIT] Setting system time via NTP...");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }
  Serial.println();
  DEBUG_PRINTLN("[INIT] System time updated");
  
  // Set up IoT Hub connection
  String iotHubHostname = String(IOT_HUB_NAME) + IOT_HUB_SUFFIX;
  mqttClient.setServer(iotHubHostname.c_str(), MQTT_PORT);
  mqttClient.setCallback(onMqttMessage);
  
  // Disable certificate validation for development (⚠️ NOT SECURE - for testing only)
  // espClient.setInsecure(); // Uncomment for self-signed certificates
  
  DEBUG_PRINTLN("[INIT] IoT Hub endpoint configured");
  DEBUG_PRINTLN("===============================================");
  DEBUG_PRINTLN();
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {
  // Check and maintain WiFi connection
  if (!wifiConnected) {
    unsigned long now = millis();
    if (now - lastWiFiAttempt >= WIFI_RECONNECT_INTERVAL) {
      connectToWiFi();
      lastWiFiAttempt = now;
    }
  } else {
    // WiFi is connected, ensure IoT Hub connection
    if (!iotHubConnected) {
      unsigned long now = millis();
      if (now - lastIoTHubAttempt >= IOT_HUB_RECONNECT_INTERVAL) {
        connectToIoTHub();
        lastIoTHubAttempt = now;
      }
    } else {
      // Both WiFi and IoT Hub connected
      mqttClient.loop();
      
      // Read sensor and send telemetry at interval
      unsigned long now = millis();
      if (now - lastTelemetrySend >= TELEMETRY_INTERVAL) {
        readAndSendTelemetry();
        lastTelemetrySend = now;
      }
    }
  }
  
  // Prevent watchdog timeout
  delay(100);
}

// ==========================================
// WIFI CONNECTION
// ==========================================

void connectToWiFi() {
  if (wifiConnected) return;
  
  DEBUG_PRINT("[WiFi] Connecting to ");
  DEBUG_PRINTLN(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttempt = millis();
  
  while (WiFi.status() != WL_CONNECTED && 
         (millis() - startAttempt) < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    DEBUG_PRINTLN("[WiFi] ✓ Connected!");
    DEBUG_PRINT("[WiFi] IP Address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINT("[WiFi] RSSI: ");
    DEBUG_PRINTLN(WiFi.RSSI());
  } else {
    wifiConnected = false;
    DEBUG_PRINTLN("[WiFi] ✗ Connection failed");
    WiFi.disconnect(true); // Turn off WiFi to save power
  }
}

// ==========================================
// IOT HUB CONNECTION
// ==========================================

void connectToIoTHub() {
  if (iotHubConnected) return;
  if (!wifiConnected) return;
  
  DEBUG_PRINTLN("[IoTHub] Connecting...");
  
  // Build connection string
  String iotHubHostname = String(IOT_HUB_NAME) + IOT_HUB_SUFFIX;
  String clientId = DEVICE_ID;
  String username = String(IOT_HUB_NAME) + ".azure-devices.net/" + DEVICE_ID + "/api-version=2021-04-12";
  String password = generateSasToken();
  
  // Connect to IoT Hub
  if (mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str())) {
    iotHubConnected = true;
    DEBUG_PRINTLN("[IoTHub] ✓ Connected!");
    
    // Subscribe to cloud-to-device messages (optional)
    String subscribeTopic = "devices/" + String(DEVICE_ID) + "/messages/devicebound/#";
    mqttClient.subscribe(subscribeTopic.c_str());
  } else {
    iotHubConnected = false;
    DEBUG_PRINT("[IoTHub] ✗ Connection failed: ");
    DEBUG_PRINTLN(mqttClient.state());
  }
}

// ==========================================
// SENSOR READING
// ==========================================

float readDistance() {
  // Send pulse to trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH, SENSOR_READ_TIMEOUT);
  
  // Calculate distance (speed of sound = 343 m/s = 0.0343 cm/us)
  // distance = (duration / 2) * 0.0343
  float distance = (duration / 2.0) * 0.0343;
  
  return distance;
}

float calculateWaterLevel() {
  // Take multiple samples and average
  float totalDistance = 0;
  int validSamples = 0;
  
  for (int i = 0; i < SENSOR_SAMPLES; i++) {
    float distance = readDistance();
    
    // Validate reading
    if (distance > MIN_TANK_DEPTH && distance < MAX_TANK_DEPTH) {
      totalDistance += distance;
      validSamples++;
    }
    
    delay(SAMPLE_DELAY);
  }
  
  if (validSamples == 0) {
    DEBUG_PRINTLN("[Sensor] ✗ No valid readings");
    return -1; // Error
  }
  
  float avgDistance = totalDistance / validSamples;
  currentDepthCm = avgDistance;
  
  // Calculate water level percentage
  // 0% when sensor reads MAX_TANK_DEPTH (empty)
  // 100% when sensor reads MIN_TANK_DEPTH (full)
  float waterLevelPercent = ((MAX_TANK_DEPTH - avgDistance) / 
                             (MAX_TANK_DEPTH - MIN_TANK_DEPTH)) * 100.0;
  
  // Clamp to 0-100%
  if (waterLevelPercent < 0) waterLevelPercent = 0;
  if (waterLevelPercent > 100) waterLevelPercent = 100;
  
  currentWaterLevelPercent = waterLevelPercent;
  
  return waterLevelPercent;
}

// ==========================================
// TELEMETRY SENDING
// ==========================================

void readAndSendTelemetry() {
  if (!iotHubConnected) {
    DEBUG_PRINTLN("[Telemetry] ✗ Not connected to IoT Hub");
    return;
  }
  
  DEBUG_PRINTLN("[Sensor] Reading water level...");
  float waterLevel = calculateWaterLevel();
  
  if (waterLevel < 0) {
    DEBUG_PRINTLN("[Sensor] ✗ Invalid reading");
    return;
  }
  
  DEBUG_PRINT("[Sensor] Depth: ");
  DEBUG_PRINT(currentDepthCm);
  DEBUG_PRINTLN(" cm");
  
  DEBUG_PRINT("[Sensor] Water Level: ");
  DEBUG_PRINT(currentWaterLevelPercent);
  DEBUG_PRINTLN(" %");
  
  // Build JSON telemetry payload
  StaticJsonDocument<256> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["depth_cm"] = round(currentDepthCm * 10) / 10.0; // 1 decimal place
  doc["water_level_percent"] = (int)round(currentWaterLevelPercent);
  doc["signal_strength"] = WiFi.RSSI();
  doc["timestamp"] = getISOTimestamp();
  
  // Serialize to string
  String payload;
  serializeJson(doc, payload);
  
  // Publish to IoT Hub
  String telemetryTopic = "devices/" + String(DEVICE_ID) + "/messages/events/";
  
  if (mqttClient.publish(telemetryTopic.c_str(), payload.c_str())) {
    DEBUG_PRINTLN("[Telemetry] ✓ Data sent successfully");
    DEBUG_PRINT("[Telemetry] Payload: ");
    DEBUG_PRINTLN(payload);
  } else {
    DEBUG_PRINTLN("[Telemetry] ✗ Failed to send");
  }
}

// ==========================================
// SAS TOKEN GENERATION
// ==========================================

String generateSasToken() {
  // For simplicity, using a pre-generated SAS token
  // In production, implement proper token generation with HMAC-SHA256
  
  // This is a placeholder - actual implementation requires:
  // 1. Creating HMAC-SHA256 hash of the connection string
  // 2. Base64 encoding the result
  // 3. Building the full SAS token
  
  // For now, return empty string (connection uses symmetric key auth)
  return String(DEVICE_KEY);
}

// ==========================================
// MQTT CALLBACK
// ==========================================

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Handle cloud-to-device messages
  DEBUG_PRINT("[MQTT] Message received on topic: ");
  DEBUG_PRINTLN(topic);
  
  // Convert payload to string
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  DEBUG_PRINT("[MQTT] Payload: ");
  DEBUG_PRINTLN(message);
  
  // Parse and handle specific commands if needed
}

// ==========================================
// UTILITIES
// ==========================================

String getISOTimestamp() {
  time_t now = time(nullptr);
  struct tm* timeinfo = gmtime(&now);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
  return String(buffer);
}

// ==========================================
// END OF FIRMWARE
// ==========================================
