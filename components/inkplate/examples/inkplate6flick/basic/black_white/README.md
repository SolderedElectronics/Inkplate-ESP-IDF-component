# Black and White

Draw basic shapes and text in 1-bit black and white mode on Soldered Inkplate 6 Flick.

## Overview

Cycles through Adafruit GFX drawing primitives in 1-bit (black & white) mode: pixels, lines, thick lines, grids, rectangles, circles, rounded rectangles, triangles, ellipses, polygons, bitmaps, and text at multiple sizes. Each shape is shown for 5 seconds. The demo ends with text that rotates indefinitely.

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

A series of screens demonstrating pixels, lines, shapes, and text in black & white across the 1024×758 px display, ending with rotating text.

## Notes

- `DELAY_MS` in `main.cpp` controls how long each shape is shown (default 5000 ms).
- 1-bit mode enables partial updates and faster refresh compared to grayscale.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
