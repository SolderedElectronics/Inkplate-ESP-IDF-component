# RTC Timer

PCF85063A RTC countdown timer example for Soldered Inkplate 6Color.

## Overview

Demonstrates how to configure and use the countdown timer feature of the PCF85063A RTC. When the timer expires, an interrupt fires and the Inkplate 6Color display is updated.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

In `main.cpp`, set the timer countdown value before building.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display updates when the RTC countdown timer expires.

## Notes

- The PCF85063A timer supports countdown intervals from fractions of a second to minutes.
- Timer interrupt is routed via GPIO39 on Inkplate 6Color.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
