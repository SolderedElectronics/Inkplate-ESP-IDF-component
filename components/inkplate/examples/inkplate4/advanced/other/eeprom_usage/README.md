# EEPROM (NVS) Usage

Non-volatile storage read/write example for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the ESP32 Non-Volatile Storage (NVS) — the ESP-IDF equivalent of Arduino EEPROM — to store data that persists across resets and power cycles. The example clears user NVS data, writes sample values, reads them back, and displays the results on screen.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Messages indicating NVS clearing, writing, and reading steps.
- A list of values read back from NVS displayed on the screen.

## Notes

- NVS data is stored in a dedicated flash partition and survives power cycles.
- Data is stored in a dedicated NVS flash partition.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
