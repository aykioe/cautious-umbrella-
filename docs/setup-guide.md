# Setup Guide

## Prerequisites
- Arduino IDE (for Pod 1)
- PlatformIO (for Pod 2)
- Node.js v18+ (for Dashboard)

## Step 1 — Flash Pod 1 (Arduino)
1. Open `pod-1-arduino/src/main.ino` in Arduino IDE
2. Install required libraries via Library Manager
3. Select your board and port, then upload

## Step 2 — Flash Pod 2 (ESP32)
1. Edit `pod-2-esp32/config/config.h` with your WiFi and dashboard IP
2. Run `pio run --target upload` from the `pod-2-esp32/` directory

## Step 3 — Run the Dashboard
1. `cd dashboard && npm install`
2. Create a `.env` file (see dashboard/README.md)
3. Run `npm run dev`
4. Open `http://localhost:3000` in your browser

## Step 4 — Verify
- Pod 2 should start sending data to the dashboard automatically
- Pod 1 outputs to Serial — open Serial Monitor at 9600 baud
