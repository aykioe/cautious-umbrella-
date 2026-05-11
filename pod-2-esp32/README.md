# Pod 2 — ESP32 Sensor Pod

Reads temperature/humidity, light/UV, and soil moisture. Sends data to the dashboard via WiFi (HTTP POST).

## Hardware
- ESP32 DevKit
- SHT31 (Temp/Humidity) — I2C
- VEML6070 (UV) — I2C
- Capacitive Soil Moisture Sensor — GPIO34

## Setup
1. Install [PlatformIO](https://platformio.org/)
2. Edit `config/config.h` with your WiFi credentials and dashboard URL
3. Run `pio run --target upload`

## Config
```cpp
// config/config.h
#define WIFI_SSID     "your_network"
#define WIFI_PASSWORD "your_password"
#define DASHBOARD_URL "http://your-dashboard-ip:3000/api/data"
#define POD_ID        "pod-2"
```
