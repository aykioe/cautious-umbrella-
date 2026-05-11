#pragma once

// WiFi Credentials
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Dashboard
#define DASHBOARD_URL "http://192.168.1.100:3000/api/data"
#define POD_ID        "pod-2"

// Sensor Pins
#define SOIL_MOISTURE_PIN 34

// Intervals
#define READ_INTERVAL_MS  5000   // Read sensors every 5 seconds
#define SEND_INTERVAL_MS  30000  // Send to dashboard every 30 seconds
