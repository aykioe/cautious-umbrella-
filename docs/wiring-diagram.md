# Wiring Diagram Reference

## Pod 1 — Arduino Nano Pin Map

| Pin | Connected To | Function |
|-----|-------------|----------|
| 3.3V | AS7341 VIN | Spectral sensor power |
| 5V | MAX485-SOIL VCC, MAX485-BUS VCC | RS485 module power |
| GND | All GND rails | Common ground |
| A4 (SDA) | AS7341 SDA | I2C data |
| A5 (SCL) | AS7341 SCL | I2C clock |
| D0 (RX) | MAX485-BUS RO | Inter-pod bus receive |
| D1 (TX) | MAX485-BUS DI | Inter-pod bus transmit |
| D2 | MAX485-BUS DE + RE (tied) | Inter-pod bus direction |
| D3 | MAX485-SOIL DE + RE (tied) | Modbus probe direction |
| D5 | MAX485-SOIL RO | Modbus probe receive (SoftwareSerial) |
| D6 | MAX485-SOIL DI | Modbus probe transmit (SoftwareSerial) |

## Pod 2 — ESP32-CAM Pin Map

| Pin | Connected To | Function |
|-----|-------------|----------|
| Vin (5V) | TP4056 OUT+, MAX485 VCC, DEVMO HV, MT3608 IN+ | 5V distribution rail |
| 3.3V | DEVMO LV | Level shifter low-voltage side |
| GND | All GND rails | Common ground |
| GPIO 2 | DEVMO LV Ch2 → HV Ch2 → MAX485 RO | RS485 receive |
| GPIO 13 | DEVMO LV Ch1 → HV Ch1 → MAX485 DI | RS485 transmit |
| GPIO 15 | DEVMO LV Ch3 → HV Ch3 → MAX485 DE+RE | RS485 direction |
| GPIO 12 | ⛔ DO NOT CONNECT | Flash voltage bootstrap |

## RS485 Bus Wiring
```
Pod 1 MAX485-BUS A  ──twisted pair──  Pod 2 MAX485 A
Pod 1 MAX485-BUS B  ──twisted pair──  Pod 2 MAX485 B
120Ω across A/B at Pod 1 end
120Ω across A/B at Pod 2 end
```

## ALOUTSNOC Probe Cable (verify against your included datasheet)
| Wire Color | Connects To |
|-----------|-------------|
| Brown | 12V (MT3608 OUT+) |
| Black | GND |
| Yellow | MAX485-SOIL A |
| Blue | MAX485-SOIL B |
