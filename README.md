# Water Level Monitor - IoT Solution with ESP32 and Azure

A complete cloud-based water level monitoring system using ESP32 DevKitV1, HC-SR04 ultrasonic sensor, and Microsoft Azure IoT services.

## 🎯 Project Overview

This project monitors water tank depth in real-time, converts it to a percentage, and displays it on a web dashboard. Data is securely transmitted to Azure IoT Hub and stored for historical analysis.

### Key Features
- ✅ Real-time water depth measurement using HC-SR04 ultrasonic sensor
- ✅ Water level percentage calculation
- ✅ Secure MQTT connection to Azure IoT Hub
- ✅ Time-series data storage in Azure Table Storage
- ✅ Professional web dashboard with real-time charts
- ✅ Historical data visualization
- ✅ Mobile-responsive design
- ✅ Error handling and automatic reconnection
- ✅ Secure credential management (no hardcoded secrets)

## 📋 Project Structure

```
water-level-monitor/
├── firmware/                          # ESP32 Arduino firmware
│   ├── esp32_water_monitor.ino       # Main firmware code
│   ├── config.h                       # Configuration constants
│   ├── secrets_example.h              # Secrets template
│   └── README.md                      # Firmware setup guide
├── azure/                             # Azure cloud setup
│   ├── iot_hub_setup.md              # IoT Hub configuration
│   ├── storage_setup.md              # Table Storage setup
│   ├── stream_analytics_setup.md     # Stream Analytics (optional)
│   └── deploy.sh                      # Deployment script
├── dashboard/                         # Web dashboard application
│   ├── app.js                        # Node.js/Express backend
│   ├── package.json                  # Dependencies
│   ├── public/
│   │   ├── index.html                # Dashboard UI
│   │   ├── css/style.css             # Styling
│   │   └── js/dashboard.js           # Frontend logic
│   ├── .env.example                  # Environment variables template
│   └── README.md                      # Dashboard setup guide
├── docs/                              # Documentation
│   ├── ARCHITECTURE.md               # System architecture
│   ├── SETUP_GUIDE.md                # Complete setup walkthrough
│   ├── HARDWARE_SETUP.md             # Wiring and connections
│   ├── API_DOCUMENTATION.md          # API endpoints
│   └── TROUBLESHOOTING.md            # Common issues & solutions
├── .gitignore                         # Git ignore rules
├── LICENSE                            # MIT License
└── README.md                          # This file
```

## 🚀 Quick Start

### 1. Hardware Setup
- Connect HC-SR04 ultrasonic sensor to ESP32 (see docs/HARDWARE_SETUP.md)
- Power the ESP32 via USB

### 2. Firmware Setup
- Follow firmware/README.md
- Configure WiFi and Azure credentials
- Upload sketch to ESP32

### 3. Azure Setup
- Follow azure/iot_hub_setup.md
- Create IoT Hub and register device
- Create Storage Table for telemetry

### 4. Web Dashboard
- Follow dashboard/README.md
- Deploy to Azure App Service
- Access via web browser

## 🔐 Security

- ✅ Device authentication via SAS tokens
- ✅ Secure MQTT over TLS/SSL
- ✅ No hardcoded secrets
- ✅ Environment-based configuration

## 📚 Documentation

Detailed guides are in the `docs/` directory.

## 📄 License

MIT License - See LICENSE file
