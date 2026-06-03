# EEPROM (NVS) Usage

Non-volatile storage read/write example for Soldered Inkplate 5.

## Overview

Demonstrates how to use the ESP32 Non-Volatile Storage (NVS) — the ESP-IDF equivalent of Arduino EEPROM — to store data that persists across resets and power cycles. The example clears user NVS data, writes sample values, reads them back, and displays the results on screen.

Keys are formatted as `"d000"` to `"d127"` to emulate byte-address access.

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

- Messages indicating NVS clearing, writing, and reading steps.
- A list of values read back from NVS displayed on the screen.

## Notes

- NVS data survives power cycles and is stored in a dedicated flash partition.
- Make sure the partition table includes an NVS partition (the default `sdkconfig.defaults` handles this).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
