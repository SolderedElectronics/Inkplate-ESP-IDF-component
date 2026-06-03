# Read Temperature

Read and display the on-board temperature sensor value on Soldered Inkplate 6.

## Overview

Reads the temperature from the TPS65186 e-paper PMIC's built-in temperature sensor and displays the result in degrees Celsius on the Inkplate 6 e-paper screen.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Current board temperature in °C displayed on the Inkplate 6 e-paper screen.

## Notes

- The TPS65186 sensor measures the temperature near the e-paper PMIC, not ambient air temperature.
- Temperature affects e-paper display performance; some waveform drivers use this reading internally.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
