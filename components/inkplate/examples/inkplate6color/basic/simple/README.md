# Simple

Basic shapes and color drawing demo for Soldered Inkplate 6Color.

## Overview

Demonstrates drawing basic shapes — filled and outline rectangles, circles, and triangles — in all seven available colors. Also renders text in each color and draws the Soldered logo bitmap in multiple colors.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Filled and outline rectangles, circles, and triangles in all seven colors.
- Text "Welcome to Inkplate 6COLOR!" printed in each color.
- Soldered logo drawn in six colors at the bottom of the screen.

## Notes

- Inkplate 6Color supports 7 colors: black, white, green, blue, red, yellow, orange.
- `display.clearDisplay()` clears only the framebuffer; call `display.display()` to update the panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
