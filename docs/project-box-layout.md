# Project Box Layout — Root Flow Sensor System

Both pods share a single enclosure. The box is divided internally into two compartments separated by a partition wall.

---

## Enclosure Overview

```
┌─────────────────────────────────────────────────────┐
│                   ROOT FLOW BOX                     │
│                                                     │
│  ┌──────────────────┐  │  ┌──────────────────────┐  │
│  │   SOIL POD       │  │  │   LEAF POD           │  │
│  │   (Nano side)    │  │  │   (ESP32-CAM side)   │  │
│  └──────────────────┘  │  └──────────────────────┘  │
│                        │                             │
└─────────────────────────────────────────────────────┘
                   internal divider
```

---

## Inside the Box

### Soil Pod Compartment (left side)
| Component | Mounted How |
|-----------|-------------|
| Arduino Nano | Soldered to proto board / Nano breakout |
| MAX485-SOIL module | Proto board — connected to D5/D6/D3 |
| MAX485-BUS module | Proto board — connected to D0/D1/D2 |
| AS7341 spectral sensor | Proto board — I2C (A4/A5) |
| 2× 100nF decoupling capacitors | Across each MAX485 VCC/GND |
| Micro USB socket (panel mount) | Mounted to box wall — power input |

### Leaf Pod Compartment (right side)
| Component | Mounted How |
|-----------|-------------|
| ESP32-CAM (AI-Thinker) | Proto board |
| DEVMO IIC Logic Level Converter | Proto board — between ESP32-CAM and MAX485 |
| MAX485 module | Proto board — connected via DEVMO shifter |
| MT3608 boost converter | Proto board — tuned to 12V |
| TP4056 USB-C charging module | Panel mount on box wall — charging port exposed |
| 18650 battery in diymore holder | Mounted to floor of compartment |

### Shared / Internal
| Item | Location |
|------|----------|
| RS485 twisted pair (A/B) | Passes through partition via grommet |
| 120Ω termination resistors | Soldered inline at each pod's MAX485 A/B pins |
| Common GND bus wire | Links both pod GND rails through partition |

---

## Outside the Box (Panel Cutouts)

### Front Face
| Cutout / Feature | Pod Side | Purpose |
|-----------------|----------|---------|
| Micro USB port | Soil Pod (left) | 5V power input for Nano |
| USB-C port (TP4056) | Leaf Pod (right) | 18650 battery charging |
| Power LED indicator | Leaf Pod (right) | Shows when ESP32-CAM is running |

### Rear Face
| Cutout / Feature | Pod Side | Purpose |
|-----------------|----------|---------|
| Cable gland (×2) | Soil Pod (left) | ALOUTSNOC probe cable exit (soil-side) |
| Cable gland (×1) | Leaf Pod (right) | RS485 bus cable exit (if pods are split later) |

### Top Face
| Cutout / Feature | Pod Side | Purpose |
|-----------------|----------|---------|
| Mesh / grille window | Leaf Pod (right) | Ventilation for ESP32-CAM and battery heat |
| AS7341 clear window | Soil Pod (left) | Light sensor must have line-of-sight to plant |

---

## Recommended Box Size

Based on component footprints, a minimum internal size of **160 × 80 × 50 mm** is recommended.
A standard **Hammond 1591XXFLBK** (171 × 121 × 55 mm) or equivalent ABS project box gives comfortable clearance for both compartments plus cable routing.
"Mustafa pls make sure its not smaller then this"
---

## Wiring Notes for Box Assembly

- Run the RS485 inter-pod twisted pair (A/B) through the partition with a small grommet or heat-shrink sleeve to prevent chafing
- Keep the MT3608 boost converter away from the 18650 battery — heat from the inductor can affect cell temperature readings
- The AS7341 should be positioned near the top of the Soil Pod compartment so the clear window cutout aligns with it
- Label all external ports with a label maker or printed overlay before sealing

