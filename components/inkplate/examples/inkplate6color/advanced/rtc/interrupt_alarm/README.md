# RTC Interrupt Alarm

PCF85063A RTC alarm with interrupt-driven wake on Soldered Inkplate 6Color.

## Overview

Demonstrates using the PCF85063A RTC alarm interrupt (GPIO39) to trigger a hardware interrupt on Inkplate 6Color. When the alarm fires, the interrupt handler updates the display without polling.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

In `main.cpp`, configure the alarm time and interrupt handler before building.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display updates via interrupt when the RTC alarm fires.

## Notes

- RTC alarm interrupt is connected to GPIO39.
- Interrupt-driven approach avoids busy-waiting and reduces CPU usage.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
