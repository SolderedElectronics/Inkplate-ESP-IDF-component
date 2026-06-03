# Touchscreen Serial

Print touch coordinates to serial for Soldered Inkplate 4TEMPERA.

## Overview

Reads touch events from the touchscreen controller and prints the number of fingers detected and their X/Y coordinates to the serial output. Also draws a reference triangle at the (0,0) corner of the display to help orient coordinates. Prints "Release" when all fingers are lifted.

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
3. Touch the screen to print finger count and coordinates.

## Expected Output

- `"1 finger X=123 Y=456"` (or 2 fingers) printed per touch event.
- `"Release"` printed when all fingers are lifted.

## Notes

- `getData()` returns the number of active touch points and fills the x/y arrays.
- Up to 2 simultaneous touch points are supported.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
