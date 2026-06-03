# Simple

Draw basic shapes and text in all six colors on Soldered Inkplate 13SPECTRA.

## Overview

Demonstrates drawing filled and outline rectangles, circles, and triangles in all six available colors of the Inkplate 13SPECTRA display. Also renders text in each color and draws the Soldered logo bitmap.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Filled and outline rectangles, circles, and triangles in all six colors.
- "Welcome to Inkplate 13SPECTRA!" printed in each color.
- Soldered logo drawn in five colors at the bottom of the screen.

## Notes

- Inkplate 13SPECTRA supports 6 colors: black, white, yellow, red, blue, green.
- `display.display()` must be called to push the framebuffer to the physical panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
