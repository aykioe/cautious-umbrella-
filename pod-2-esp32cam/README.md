# Pod 2 — ESP32-CAM Leaf Pod

Receives soil and spectral data from Pod 1 over the inter-pod RS485 bus, connects to Wi-Fi, and sends Telegram alerts when thresholds are exceeded.

## Hardware
- ESP32-CAM AI-Thinker (built-in USB-C TTL — no FTDI needed)
- MAX485 module — inter-pod bus (GPIO 13=DI, GPIO 2=RO, GPIO 15=DE/RE via DEVMO shifter)
- DEVMO IIC Logic Level Converter (LV=3.3V side → ESP32-CAM; HV=5V side → MAX485)
- MT3608 Boost Converter (tuned to 12V — powers probe VCC and RS485 bus)
- TP4056 USB-C + 18650 battery (in diymore holder)

## GPIO Pin Assignments
| GPIO | Function |
|------|----------|
| GPIO 2  | MAX485 RO (receive) → via DEVMO Ch2 |
| GPIO 13 | MAX485 DI (transmit) → via DEVMO Ch1 |
| GPIO 15 | MAX485 DE/RE (direction) → via DEVMO Ch3 |
| GPIO 12 | ⛔ RESERVED — do not connect (flash voltage bootstrap) |

## Libraries Required (PlatformIO)
See `platformio.ini` — installs automatically on build.

## Setup
1. Edit `config/config.h` with your Wi-Fi credentials and Telegram token/chat ID
2. Plug in USB-C — hold BOOT button while connecting, release after 1 second
3. Run `pio run --target upload` from this directory
4. Press EN (reset) button after upload completes

## Telegram Alerts
Alerts fire when any reading crosses a threshold defined in `src/thresholds.h`.
Example: soil moisture below 30% → "⚠️ Root Flow: Soil moisture low (28%)"
