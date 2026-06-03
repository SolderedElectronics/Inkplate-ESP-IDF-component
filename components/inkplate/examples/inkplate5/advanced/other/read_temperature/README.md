# Read Temperature

Read temperature from the on-board TPS65186 PMIC sensor on Soldered Inkplate 5.

## Overview

Demonstrates how to read temperature data from the on-board sensor integrated inside the TPS65186 e-paper PMIC. The measured value is shown on the display alongside a thermometer icon and updated every 10 seconds.

Note: this sensor is intended primarily for internal compensation and basic monitoring — it is not a high-accuracy sensor.

## Hardware Required

- Soldered Inkplate 5
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Thermometer icon with temperature in °C displayed on screen, updated every 10 seconds.

## Notes

- `readTemperature()` returns an `int8_t` value in degrees Celsius.
- The TPS65186 PMIC sensor is intended for system monitoring and waveform compensation, not precision measurement.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
