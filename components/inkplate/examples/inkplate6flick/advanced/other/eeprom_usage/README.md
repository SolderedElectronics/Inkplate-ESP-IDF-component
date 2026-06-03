# EEPROM Usage

Store and retrieve persistent data using NVS on Soldered Inkplate 6 Flick.

## Overview

Demonstrates how to use ESP32 Non-Volatile Storage (NVS) as an EEPROM equivalent. A counter value is written to NVS on each run, read back, incremented, and displayed on the Inkplate 6 Flick e-paper screen. The value persists across power cycles and resets.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The persistent counter value is displayed and increments on each power cycle or reset.

## Notes

- NVS data survives deep sleep and power-off.
- NVS write endurance is limited; avoid writing on every loop iteration.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
