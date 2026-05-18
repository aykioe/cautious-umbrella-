# Pod 1 — Arduino Nano (Soil Pod)

## Hardware
- Arduino Nano (ATmega328P)
- ALOUTSNOC RS485 Modbus Soil Sensor (moisture, temp, EC, pH) — SoftwareSerial via MAX485
- waterproof DS18B20 probe
- TCS34725 RGB Light Sensor Breakout
- Capacitive Soil Moisture Sensor v1.2
- AS7341 11-Channel Spectral Sensor — I2C (A4=SDA, A5=SCL)
- MAX485-SOIL module — SoftwareSerial (D5=RO, D6=DI, D3=DE/RE)
- MAX485-BUS module — HardwareSerial (D0=RX, D1=TX, D2=DE/RE)

## Libraries Required (Arduino IDE Library Manager)
- `Adafruit AS7341`
- `Adafruit BusIO`
- `ModbusMaster` by Doc Walker

## Setup
1. Install libraries above in Arduino IDE
2. Edit `config/config.h` — set baud rates and pin assignments if changed
3. **Disconnect D0 and D1 from MAX485-BUS before uploading**
4. Select board: Arduino Nano, Processor: ATmega328P
5. Upload `src/main.ino`
6. Reconnect D0/D1 after upload

## Serial Output (9600 baud)
```
[SOIL] moisture:45 temp:22.3 ec:1.2 ph:6.8
[SPEC] F1:120 F2:340 F3:512 F4:480 F5:390 F6:310 F7:220 F8:180 NIR:95 CLR:1024
[BUS]  TX OK
```
