# Fastest Display Refreshes

Speed up consecutive partial refreshes by keeping the e-paper panel powered on — Inkplate 5.

## Overview

Demonstrates how to achieve faster partial updates by keeping the e-paper panel power enabled during repeated refreshes using `einkOn()`. Normally, panel power is cycled before and after each refresh. Keeping it on eliminates this overhead for faster successive `partialUpdate()` calls. The example scrolls text across the screen to visualize the speed improvement.

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

- Full refresh at startup.
- Smooth scrolling text using faster partial refreshes.

## Notes

- Partial update is supported only in 1-bit (black & white) mode.
- Keeping the panel powered on increases power consumption.
- Always call `einkOff()` before entering deep sleep or long idle periods.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
