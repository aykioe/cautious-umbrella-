# Pod 1 — Arduino Sensor Pod

Reads temperature/humidity, light/UV, and soil moisture. Outputs data over Serial.

## Hardware
- Arduino Uno or Nano
- DS18B20 (Temp/Humidity) — Pin 2
- BH1750 (Light) — I2C (SDA/SCL)
- Capacitive Soil Moisture Sensor — A0

## Setup
1. Install Arduino IDE
2. Install libraries: `DHT sensor library`, `BH1750`, `Wire`
3. Open `src/main.ino` and upload to your board

## Serial Output
Data is printed every 5 seconds in CSV format:
```
timestamp,temp_c,humidity_pct,light_lux,soil_moisture_pct
```
