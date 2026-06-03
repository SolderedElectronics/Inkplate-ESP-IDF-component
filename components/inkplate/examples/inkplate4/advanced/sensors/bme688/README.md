# BME688 Environmental Sensor

Read temperature, humidity, pressure, gas resistance, and altitude from the built-in BME688 sensor on Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the built-in Bosch BME688 sensor on Inkplate 4TEMPERA. Values are read and displayed on the e-paper screen once per second using partial updates; a full refresh is forced periodically to limit ghosting.

Measurements:
- Temperature (°C, with adjustable calibration offset)
- Relative humidity (%)
- Barometric pressure (hPa)
- Gas resistance (mΩ)
- Estimated altitude (m, derived from pressure)

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Temperature, humidity, pressure, gas resistance, and altitude displayed and updated every second.

## Notes

- Partial update is supported only in 1-bit (black & white) mode.
- Altitude is estimated from pressure and is not a precision measurement.
- Adjust `TEMP_OFFSET` if temperature reads consistently high or low.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
