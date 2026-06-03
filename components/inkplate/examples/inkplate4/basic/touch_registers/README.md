# Touch Registers

Raw touchscreen register dump for Soldered Inkplate 4TEMPERA.

## Overview

Reads the 8 raw touchscreen controller registers each time a touch is detected and prints them bit-by-bit to the serial output. Useful for debugging touch hardware or understanding the low-level register layout. Also draws a reference triangle at the (0,0) corner of the display to help orient coordinates.

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

## How to Use

1. Flash and power on.
2. Open a serial monitor at the default baud rate.
3. Touch the screen to print register values.

## Expected Output

8 lines of 8-bit binary register values printed to serial on each touch event, once per second.

## Notes

- `getRawData()` fills an 8-byte array with the raw register contents.
- Registers are printed once per second when a touch is detected.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
