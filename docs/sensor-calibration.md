# Sensor Calibration Notes

## AS7341 Spectral Sensor
The AS7341 does not require physical calibration for relative readings.
- **Integration time**: `ATIME=100`, `ASTEP=999` gives a ~280 ms integration — suitable for most indoor plant light conditions
- **Gain**: Start at `AS7341_GAIN_256X`. If channels saturate (read 65535), reduce gain
- **Saturation check**: Any channel reading 65535 means that wavelength band is overexposed — reduce ATIME or gain in `config/config.h`

## ALOUTSNOC RS485 Modbus Probe
The probe ships factory-calibrated. Field notes:
- **Moisture**: Insert fully into moist soil. Allow 60 seconds to stabilise before reading
- **EC (electrical conductivity)**: Readings above 3.0 mS/cm indicate salt buildup — consider flushing
- **pH**: Allow 30–60 seconds per reading. Rinse probe with distilled water between test soils
- **Default Modbus address**: 0x01. Default baud: 9600. Do not change unless running multiple probes
- **A/B polarity**: If readings are garbage or timeout, swap the A and B wires — polarity is not labelled consistently across batches
