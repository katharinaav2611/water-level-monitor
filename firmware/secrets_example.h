#ifndef SECRETS_H
#define SECRETS_H

// ==========================================
// ⚠️  IMPORTANT SECURITY NOTE  ⚠️
// ==========================================
// This file contains sensitive credentials.
// NEVER commit secrets.h to version control!
// Always use this .example file as a template.
// ==========================================

// ==========================================
// WIFI CONFIGURATION
// ==========================================

// Replace with your WiFi network name and password
const char* WIFI_SSID = "YOUR_WIFI_NETWORK_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ==========================================
// AZURE IOT HUB CONFIGURATION
// ==========================================

// Your IoT Hub name (without .azure-devices.net)
// Example: "my-water-monitor-hub"
const char* IOT_HUB_NAME = "your-iot-hub-name";

// Device ID (should match the device name registered in IoT Hub)
// Example: "esp32-water-monitor"
const char* DEVICE_ID = "esp32-water-monitor";

// Device Primary Key
// Get this from:
// 1. Azure Portal → IoT Hub → Devices → Your Device
// 2. Copy the "Primary key" value
// Example: "5+mE9Yz7kL8vQ3xP2nB1cD4fR6sT9uV0wX1yZ2aB3cD4="
const char* DEVICE_KEY = "YOUR_DEVICE_PRIMARY_KEY";

// ==========================================
// HOW TO GET AZURE CREDENTIALS
// ==========================================
/*

1. CREATE IOT HUB (if not already created):
   - Azure Portal → Create a resource → Search "IoT Hub"
   - Fill in: Resource group, Region, IoT Hub name
   - Standard tier (S1 recommended for small projects)
   - Click "Review + create" → "Create"

2. REGISTER DEVICE:
   - Go to your IoT Hub → Devices → "Add Device"
   - Device ID: "esp32-water-monitor"
   - Authentication type: Symmetric key (auto-generate)
   - Click "Save"

3. GET CONNECTION DETAILS:
   - Click on your device in the Devices list
   - Copy the following:
     - Device ID (e.g., "esp32-water-monitor")
     - Primary key (e.g., "5+mE9Yz7kL8vQ3xP2nB1cD4fR6sT9uV0wX1yZ2aB3cD4=")
     - IoT Hub name from the hostname (e.g., "my-water-monitor-hub" from "my-water-monitor-hub.azure-devices.net")

4. FILL IN THIS FILE:
   - Replace YOUR_WIFI_NETWORK_NAME with your WiFi SSID
   - Replace YOUR_WIFI_PASSWORD with your WiFi password
   - Replace your-iot-hub-name with your IoT Hub name (without .azure-devices.net)
   - Replace YOUR_DEVICE_PRIMARY_KEY with the Primary key from step 3

5. RENAME FILE:
   - Rename "secrets_example.h" to "secrets.h"
   - secrets.h is in .gitignore and won't be committed

*/

#endif // SECRETS_H
