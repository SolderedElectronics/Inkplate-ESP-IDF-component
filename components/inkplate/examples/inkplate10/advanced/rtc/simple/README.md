# RTC Simple

Basic PCF85063A RTC set and read example for Soldered Inkplate 10.

## Overview

Demonstrates how to set the time and date on the PCF85063A real-time clock and read it back. The current time is displayed on the Inkplate 10 e-paper screen and updated periodically.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

In `main.cpp`, set the initial time and date values before building.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Current time and date displayed on the Inkplate 10 screen, updating periodically.

## Notes

- The RTC continues to run on its backup supply when the main power is off.
- Reset the time by modifying the initial values in `main.cpp` and reflashing.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
