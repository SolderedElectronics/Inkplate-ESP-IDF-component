# Black and White

Black & white drawing demo using Adafruit GFX on Soldered Inkplate 10.

## Overview

Demonstrates basic drawing and text rendering in 1-bit (black & white) mode using Adafruit GFX-compatible functions. Draws pixels, lines, shapes, polygons, and text in multiple sizes, and renders a monochrome bitmap (Soldered logo).

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

A sequence of graphics demos cycling through: pixels, lines, rectangles, circles, triangles, rounded rectangles, ellipses, polygons, bitmap, and text rendering. Final section continuously rotates and displays text.

## Notes

- 1-bit mode supports only BLACK and WHITE.
- Avoid refreshing the full display too often; use partial update for frequent updates.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
