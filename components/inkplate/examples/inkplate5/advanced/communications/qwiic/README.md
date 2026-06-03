# Qwiic / I2C Scanner

Scan the I2C bus for connected Qwiic/I2C devices and display detected addresses on Inkplate 5.

## Overview

Scans the I2C bus and shows detected device addresses on both the e-paper display and ESP log output. Useful for validating wiring and confirming device communication. The scan repeats every 5 seconds.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- Qwiic/I2C device (optional — scanner works with no devices connected too)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display lists detected I2C device addresses.
- ESP log shows scanning progress and found addresses.

## Notes

- Valid I2C addresses range from 0x01 to 0x7E.
- Scan repeats every 5 seconds.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
