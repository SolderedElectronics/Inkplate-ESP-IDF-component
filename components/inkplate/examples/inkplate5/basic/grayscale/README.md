# Grayscale Drawing Showcase

Comprehensive grayscale graphics demo for Soldered Inkplate 5 using 8-shade grayscale mode.

## Overview

Cycles through Adafruit GFX drawing primitives in 3-bit grayscale mode (8 shades, 0 = black, 7 = white). Each shape is shown for 5 seconds. The demo ends with rotating text.

Drawing primitives demonstrated:
- Pixels, lines, thick lines, gradient lines
- Grids
- Rectangles (outlined/filled) and rounded rectangles
- Circles (outlined/filled)
- Triangles (outlined/filled)
- Grayscale bitmap images
- Text in different sizes and shades

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

A series of screens demonstrating pixels, lines, shapes, and text in 8-shade grayscale, followed by a continuous rotation demo.

## Notes

- Grayscale mode values: 0 (black) through 7 (white).
- `DELAY_MS` controls how long each shape is shown (default: 5000 ms).
- `display.display()` must be called to update the physical e-paper panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
