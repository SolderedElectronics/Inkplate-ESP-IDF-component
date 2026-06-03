# Qwiic

Use the Qwiic (I2C) connector on Soldered Inkplate 6.

## Overview

Demonstrates how to communicate with a Qwiic-compatible I2C sensor or module connected to the Inkplate 6 Qwiic port. Scans the I2C bus for connected devices and prints found addresses to the display.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- Qwiic-compatible I2C module

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

I2C addresses of detected Qwiic devices printed on the Inkplate 6 e-paper display.

## Notes

- Inkplate 6 Qwiic connector uses the standard 3.3 V I2C bus.
- Multiple Qwiic devices can be chained as long as addresses do not conflict.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
