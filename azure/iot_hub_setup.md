# Azure IoT Hub Setup Guide

This guide walks you through creating and configuring an Azure IoT Hub for your water level monitoring system.

## Prerequisites

- Azure subscription (free tier available)
- Azure Portal access
- Azure CLI (optional, for automation)

## Step 1: Create an Azure IoT Hub

### Via Azure Portal

1. **Go to Azure Portal:** https://portal.azure.com
2. **Create a resource:**
   - Click "Create a resource" (+ icon)
   - Search for "IoT Hub"
   - Click "IoT Hub" → "Create"

3. **Fill in the details:**
   - **Subscription:** Select your subscription
   - **Resource group:** Create new or select existing
     - Example: `water-monitor-rg`
   - **IoT Hub name:** Create a unique name
     - Example: `water-monitor-hub-001`
     - This becomes: `water-monitor-hub-001.azure-devices.net`
   - **Region:** Select closest to your location
     - Example: `East US` or `Europe West`

4. **Select Tier and Scale:**
   - **Tier:** Standard (S1)
   - **Number of units:** 1 (sufficient for small projects)
   - Cost: ~$25/month for S1 tier

5. **Review + Create:**
   - Click "Review + create"
   - Review settings
   - Click "Create"
   - Wait for deployment (2-5 minutes)

### Via Azure CLI (Alternative)

```bash
# Create resource group
az group create --name water-monitor-rg --location eastus

# Create IoT Hub
az iot hub create \
  --resource-group water-monitor-rg \
  --name water-monitor-hub-001 \
  --sku S1 \
  --unit 1
```

## Step 2: Register Your ESP32 Device

### Via Azure Portal

1. **Navigate to your IoT Hub:**
   - Azure Portal → Resource groups → water-monitor-rg
   - Select your IoT Hub → "IoT devices"

2. **Add a device:**
   - Click "New" (or "+ Add Device")

3. **Create device identity:**
   - **Device ID:** `esp32-water-monitor`
   - **Authentication type:** Symmetric key
   - **Auto-generate keys:** ✓ Checked
   - **Connect this device to an IoT hub:** Enabled
   - Click "Save"

4. **Get device credentials:**
   - Click on the created device
   - Copy the following values to your `firmware/secrets.h`:
     - **Device ID:** (e.g., `esp32-water-monitor`)
     - **Connection string – primary key:** Copy only the key portion
       - Format: `HostName=hub-name.azure-devices.net;DeviceId=device-id;SharedAccessKey=KEY`
       - Extract the `SharedAccessKey` value

### Via Azure CLI

```bash
# Register device
az iot hub device-identity create \
  --hub-name water-monitor-hub-001 \
  --device-id esp32-water-monitor \
  --auth-method shared_private_key

# Show device connection string
az iot hub device-identity connection-string show \
  --hub-name water-monitor-hub-001 \
  --device-id esp32-water-monitor
```

## Step 3: Configure Message Routing (Optional)

Route device messages to Azure Storage for persistent storage.

### Create Storage Account

1. **Create Storage Account:**
   - Azure Portal → "Create a resource" → "Storage account"
   - Fill in details (same resource group and region)
   - Create

2. **Create Container:**
   - Go to Storage account → "Containers"
   - Create new container: `water-monitor-data`
   - Public access: Private

### Set Up Message Routing

1. **In your IoT Hub:**
   - "Message routing" → "Endpoints"
   - Click "+ Add" → "Storage"
   - Fill in details:
     - **Endpoint name:** `water-data-storage`
     - **Storage account:** Select your storage account
     - **Container:** `water-monitor-data`
     - **File name format:** `{iothub}/{partition}/{YYYY}/{MM}/{DD}/{HH}/{mm}/{ss}.json`

2. **Create Route:**
   - "Message routing" → "Routes"
   - Click "+ Add" → "Storage"
   - Fill in details:
     - **Name:** `water-data-route`
     - **Endpoint:** `water-data-storage`
     - **Query:** `true` (all messages)
     - Click "Save"

## Step 4: Test Your Connection

### Monitor Device Messages

1. **In Azure Portal:**
   - Your IoT Hub → "Built-in endpoints"
   - Click "Events (messages/events)"
   - Click "Start monitoring"

2. **Upload firmware to ESP32** (see `firmware/README.md`)

3. **Verify messages arriving:**
   - You should see telemetry messages from your device
   - Example:
   ```json
   {
     "deviceId": "esp32-water-monitor",
     "depth_cm": 45.2,
     "water_level_percent": 45,
     "signal_strength": -65,
     "timestamp": "2026-05-11T10:30:45Z"
   }
   ```

### Via Azure CLI

```bash
# Monitor messages (requires Azure CLI extension)
az iot hub monitor-events \
  --hub-name water-monitor-hub-001 \
  --device-id esp32-water-monitor
```

## Step 5: Get Connection Details for Dashboard

1. **IoT Hub Connection String:**
   - Azure Portal → Your IoT Hub → "Shared access policies"
   - Select "service" policy
   - Copy "Primary connection string"
   - Save to `dashboard/.env` as `IOTHUB_CONNECTION_STRING`

2. **Storage Connection String:**
   - Azure Portal → Your Storage account → "Access keys"
   - Copy "Connection string" (key1)
   - Save to `dashboard/.env` as `STORAGE_CONNECTION_STRING`

## Security Best Practices

✅ **Do:**
- Use Symmetric keys for simple scenarios
- Enable TLS/SSL (MQTT port 8883)
- Store credentials in environment variables
- Use Azure Key Vault for production secrets
- Rotate keys periodically
- Use minimal permissions (least privilege)

❌ **Don't:**
- Hardcode connection strings in code
- Use primary keys in client applications
- Disable TLS/SSL
- Share keys via email or chat
- Use the same key for multiple devices

## Troubleshooting

### Device Connection Fails
- **Check device key:** Verify it matches exactly in `secrets.h`
- **Check device status:** Portal → Device → Properties → Connection state
- **Check time sync:** ESP32 system time must be correct for TLS/SSL

### No Messages Received
- **Verify firmware:** Check serial output (115200 baud)
- **Check WiFi:** Ensure ESP32 is connected to internet
- **Check protocol:** IoT Hub expects MQTT on port 8883
- **Check username:** Should be: `hub-name.azure-devices.net/device-id/api-version=2021-04-12`

### Authentication Errors
- **Invalid SAS token:** Ensure device key is correct and base64 encoded
- **Time sync issues:** Update ESP32 system time via NTP
- **Expired keys:** Generate new keys if > 1 year old

## Cost Estimation

- **IoT Hub (S1):** ~$25/month
  - Free tier available for testing (8,000 messages/day)
- **Storage Account:** ~$0.50/month (small amount of data)
- **App Service:** ~$13-55/month (B1 tier for web dashboard)
- **Total:** ~$40-85/month (production)
- **Free tier:** Completely free for testing

## Next Steps

1. [Set up Azure Table Storage](storage_setup.md)
2. [Deploy web dashboard](../dashboard/README.md)
3. [Configure Stream Analytics](stream_analytics_setup.md) (optional)

## References

- [Azure IoT Hub Documentation](https://docs.microsoft.com/en-us/azure/iot-hub/)
- [Azure IoT Hub MQTT Support](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support)
- [Azure IoT Device SDKs](https://github.com/Azure/azure-iot-sdks)
- [Azure Pricing Calculator](https://azure.microsoft.com/en-us/pricing/calculator/)
