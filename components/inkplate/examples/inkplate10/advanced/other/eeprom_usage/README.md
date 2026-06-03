# EEPROM Usage

Persistent data storage using EEPROM (NVS) on Soldered Inkplate 10.

## Overview

Demonstrates how to use the built-in EEPROM on Inkplate 10 to store data that persists across resets and power cycles. The example shows how to safely clear, write, and read user data from EEPROM while respecting reserved address ranges used by the Inkplate driver.

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

- EEPROM user data cleared.
- Sample data written and then read back, displayed on screen.

## Notes

- EEPROM is backed by ESP32 NVS (non-volatile storage).
- Lower address ranges are reserved by the Inkplate driver; do not write to them.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
