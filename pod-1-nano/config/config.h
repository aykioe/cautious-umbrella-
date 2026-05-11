#pragma once

// ── Inter-pod RS485 Bus (HardwareSerial) ──
#define BUS_BAUD        9600
#define BUS_DE_RE_PIN   2      // D2: tied DE + RE on MAX485-BUS

// ── ALOUTSNOC Modbus Probe (SoftwareSerial) ──
#define MODBUS_RO_PIN   5      // D5: RO on MAX485-SOIL
#define MODBUS_DI_PIN   6      // D6: DI on MAX485-SOIL
#define MODBUS_DE_PIN   3      // D3: DE+RE tied on MAX485-SOIL
#define MODBUS_BAUD     9600
#define MODBUS_ADDR     0x01   // Default Modbus device address

// ── AS7341 Spectral Sensor (I2C) ──
// SDA = A4, SCL = A5 (Arduino Nano hardware I2C — no defines needed)
#define AS7341_ATIME    100
#define AS7341_ASTEP    999

// ── Timing ──
#define PROBE_READ_INTERVAL_MS   5000   // Poll Modbus probe every 5 s
#define BUS_SEND_INTERVAL_MS    10000   // Send to Leaf Pod every 10 s
