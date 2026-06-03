# Fastest Display Refreshes

Speed up consecutive partial updates by keeping the e-paper panel powered on, on Soldered Inkplate 10.

## Overview

Demonstrates how to reduce latency between successive partial updates by keeping the e-paper panel power enabled (`einkOn()`) during repeated refreshes. The example scrolls text across the display using rapid partial updates while the panel stays powered.

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

- A full refresh is performed once at startup.
- Text scrolls across the display using fast partial updates.

## Notes

- Partial update works only in 1-bit (black & white) mode.
- Call `einkOn()` before the update loop and `einkOff()` when done to save power.
- Perform a full refresh periodically to prevent ghosting.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
