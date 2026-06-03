# Black White Red Drawing Showcase

Adafruit GFX drawing showcase for Inkplate 2 using the full tri-color palette (black, white, red).

## Overview

Cycles through a wide range of Adafruit GFX-compatible drawing primitives, each drawn into the framebuffer and pushed to the panel with a full refresh:

- Text with shadow
- Single pixels and random pixels
- Lines (thick, horizontal, vertical, random)
- Grids
- Rectangles (outlined/filled) and rounded rectangles
- Circles (outlined/filled)
- Triangles (outlined/filled)
- Ellipses (outlined/filled)
- Polygons (outlined/filled)

After the drawing demo, the sketch continuously rotates the display (0°/90°/180°/270°) and redraws text at each orientation.

## Hardware Required

- Soldered Inkplate 2
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display cycles through drawing demonstrations with a short pause between each step, then enters a continuous rotation loop.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
