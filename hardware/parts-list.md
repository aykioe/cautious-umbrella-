# Parts List — Filum Vitae Sensor System

## Pod 1 — Soil Pod (Arduino Nano)
| Qty | Part | Purpose | Approx. Cost | Source |
|-----|------|---------|--------------|--------|
| 1 | Arduino Nano (ATmega328P) | Soil pod microcontroller | $4–8 | Amazon |
| 1 | ALOUTSNOC RS485 Modbus Soil Sensor | Moisture, temp, EC, pH via RS485 Modbus RTU | $15–25 | Amazon |
| 1 | AS7341 11-Channel Spectral Sensor | Full visible spectrum + NIR (415–680 nm) | $8–12 | Amazon |
| 2 | SJZBIN MAX485 Transceiver Module | One for Modbus probe; one for inter-pod bus | $1–2 ea. | Amazon (SJZBIN 5PCS pack) |
| 2 | 100 nF ceramic capacitor | MAX485 decoupling (one per module) | <$0.10 ea. | Local / kit |
| 1 | Breadboard or proto board (half-size) | Mounting | $2–4 | Amazon |
| — | Jumper wires (M-M, M-F) | Connections | $3–5 set | Amazon |

## Pod 2 — Leaf Pod (ESP32-CAM)
| Qty | Part | Purpose | Approx. Cost | Source |
|-----|------|---------|--------------|--------|
| 1 | ESP32-CAM (Thinker, built-in USB-C TTL) | Leaf pod MCU + Wi-Fi. Built-in TTL = no FTDI needed. | $8–14 | Amazon (ESP32 Cam kit) |
| 1 | SJZBIN MAX485 Transceiver Module | Inter-pod RS485 bus | $1–2 | Amazon (SJZBIN 5PCS pack) |
| 1 | DEVMO IIC I2C Logic Level Converter (3.3V↔5V) | Protect ESP32-CAM GPIO from 5V MAX485 signals | $1–3 | Amazon (DEVMO listing) |
| 1 | MT3608 DC-DC Boost Module (12V output) | Powers RS485 bus + probe VCC | $1–3 | Amazon |
| 2 | 120 Ω resistors | RS485 bus termination (one per pod end) | <$0.10 ea. | Local / kit |
| 1 | Breadboard or proto board (half-size) | Mounting | $2–4 | Amazon |
| — | 2-wire twisted pair cable (≥ 1 m) | RS485 A/B inter-pod bus | $3–5/m | Amazon |

## Shared / Power
| Qty | Part | Purpose | Approx. Cost |
|-----|------|---------|--------------|
| 1 | diymore 18650 Battery Holder | Holds 18650 cell for Leaf Pod | $1–3 |
| 1 | 18650 LiPo Cell (3.7V) | Leaf Pod battery | $4–8 |
| 1 | diymore TP4056 USB-C Module (from 10PCS pack) | Charges 18650; provides 5V output to Leaf Pod | $1–2 |
| 1 | Micro USB 5V power supply (≥ 500 mA) | Soil Pod power | $5–8 |

## Bench / Test Tools (not in final circuit)
| Part | Purpose |
|------|---------|
| diymore USB Tester | Measure USB supply voltage and current during testing |
| diymore Type-C Meter (Voltage / Current) | Monitor TP4056 charge current and Leaf Pod draw |
