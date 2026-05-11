#pragma once

// ── Wi-Fi ──
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ── Telegram ──
#define TELEGRAM_TOKEN  "YOUR_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_CHAT_ID"

// ── RS485 Bus GPIO (via DEVMO level shifter) ──
#define RS485_DI_PIN    13     // Transmit data to bus
#define RS485_RO_PIN    2      // Receive data from bus
#define RS485_DE_PIN    15     // Direction control (DE + RE tied)
#define RS485_BAUD      9600

// !! GPIO 12 is reserved — flash voltage bootstrap — never connect !!

// ── Timing ──
#define RECEIVE_TIMEOUT_MS   15000   // Alert if no data received in 15 s
#define TELEGRAM_COOLDOWN_MS 60000   // Minimum time between same alert type
