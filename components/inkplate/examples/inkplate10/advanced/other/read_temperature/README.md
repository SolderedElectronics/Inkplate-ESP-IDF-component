# Read Temperature

Read on-board temperature from the TPS65186 PMIC on Soldered Inkplate 10.

## Overview

Demonstrates how to read temperature data from the on-board temperature sensor integrated inside the TPS65186 e-paper PMIC. This sensor is primarily intended for internal compensation and basic monitoring.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Temperature value (°C) read from the PMIC displayed on screen or printed to the serial monitor.

## Notes

- The TPS65186 temperature sensor is a basic internal sensor and is not intended for precision measurements.
- Accuracy is sufficient for display compensation but not for environmental monitoring.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
