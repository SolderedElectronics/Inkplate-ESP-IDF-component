# Qwiic

Connect and communicate with Qwiic (I2C) devices on Soldered Inkplate 10.

## Overview

Demonstrates how to use the Qwiic connector on Inkplate 10 to interface with I2C peripherals. The example scans the I2C bus for connected Qwiic devices and displays the found addresses on the e-paper screen.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- Qwiic-compatible I2C device

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

I2C addresses of detected Qwiic devices displayed on the Inkplate 10 screen.

## Notes

- The Qwiic connector uses the standard I2C bus (3.3 V, SDA/SCL).
- Multiple Qwiic devices can be chained on the same connector.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
