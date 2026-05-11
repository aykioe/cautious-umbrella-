# Software Setup Guide

## Prerequisites
- Arduino IDE 2.x (for Pod 1 — Nano)
- PlatformIO (for Pod 2 — ESP32-CAM)
- Node.js v18+ (for Dashboard)

## Step 1 — Install Arduino Libraries (Pod 1)
Open Arduino IDE → Tools → Manage Libraries, install:
- `Adafruit AS7341`
- `Adafruit BusIO`
- `ModbusMaster` by Doc Walker

## Step 2 — Flash Pod 1 (Nano)
1. **Disconnect D0 and D1 wires from MAX485-BUS before plugging in USB**
2. Open `pod-1-nano/src/main.ino` in Arduino IDE
3. Board: Arduino Nano | Processor: ATmega328P (Old Bootloader if needed)
4. Upload
5. Reconnect D0 and D1 after upload

## Step 3 — Configure Pod 2 (ESP32-CAM)
1. Edit `pod-2-esp32cam/config/config.h`:
   - Set `WIFI_SSID` and `WIFI_PASSWORD`
   - Set `TELEGRAM_TOKEN` and `TELEGRAM_CHAT_ID`
2. To get a Telegram bot token: message `@BotFather` on Telegram → `/newbot`
3. To get your chat ID: message `@userinfobot` on Telegram

## Step 4 — Flash Pod 2 (ESP32-CAM)
1. `cd pod-2-esp32cam`
2. Hold the BOOT button on the ESP32-CAM while plugging in USB-C
3. Release BOOT after 1 second
4. Run: `pio run --target upload`
5. Press the EN (reset) button after upload completes

## Step 5 — Run the Dashboard
```bash
cd dashboard
npm install
cp .env.example .env   # edit with your settings
npm run dev
```
Open `http://localhost:3000`

## Step 6 — Verify
- Pod 1 Serial Monitor (9600 baud): should show soil and spectral readings
- Pod 2 Serial Monitor (115200 baud): should show received data + WiFi connected
- Telegram: send `/status` to your bot to get a live reading
