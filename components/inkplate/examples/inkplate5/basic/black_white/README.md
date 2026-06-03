# Black & White Drawing Showcase

Comprehensive black & white graphics demo for Soldered Inkplate 5.

## Overview

Cycles through Adafruit GFX drawing primitives in 1-bit (black & white) mode. Each shape is shown for 5 seconds. The demo ends with rotating text.

Drawing primitives demonstrated:
- Pixels (single and random)
- Lines (normal, thick, horizontal, vertical, random)
- Grids
- Rectangles (outlined/filled) and rounded rectangles
- Circles (outlined/filled)
- Triangles (outlined/filled)
- Ellipses (outlined/filled)
- Bitmaps
- Text at multiple sizes

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

A series of screens demonstrating pixels, lines, shapes, and text in black & white, followed by a continuous rotation demo.

## Notes

- `DELAY_MS` controls how long each shape is shown (default: 5000 ms).
- `display.display()` must be called to update the physical e-paper panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
