# Schematic Notes and Bus Topology

## RS485 Bus Topology

Two independent RS485 channels exist on Pod 1 (Nano):

```
[ALOUTSNOC Probe] ──A/B──> [MAX485-SOIL] ──SoftSerial D5/D6/D3──> [Nano]
                                                                       |
                                                                  [Nano D0/D1/D2]
                                                                       |
                                                                [MAX485-BUS]
                                                                       |
                                                               A/B twisted pair
                                                                       |
                                                               [MAX485 Leaf Pod]
                                                                       |
                                                          [DEVMO Level Shifter]
                                                                       |
                                                              [ESP32-CAM GPIO]
```

## I2C Bus (Pod 1 — Nano)
- A4 = SDA, A5 = SCL
- Only device: AS7341 at address 0x39

## Power Distribution (Leaf Pod)
```
USB-C charger → TP4056 → 18650 battery
                TP4056 OUT+ → ESP32-CAM Vin (5V rail)
                ESP32-CAM Vin → MAX485 VCC
                ESP32-CAM Vin → DEVMO HV
                ESP32-CAM Vin → MT3608 IN+
                MT3608 OUT+ (12V) → Soil probe VCC + RS485 bias
```

## Notes
- GPIO 12 on ESP32-CAM is a bootstrapping pin — leave floating always
- Disconnect D0/D1 from MAX485-BUS before uploading to Nano
- MT3608 must be tuned to 12.0V ± 0.2V before connecting to probe
