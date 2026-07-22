# Full Screen Colors

Fill the Soldered Inkplate 7 screen with six vertical color bars.

## Overview

Draws six full-height vertical bars across the display, one per supported color (black, white, yellow, red, blue, green), then calls `display()` to push the framebuffer to the screen. Useful as a quick visual test of the panel's color rendering.

## Hardware Required

- Soldered Inkplate 7
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate7**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Six full-height vertical bars in: black, white, yellow, red, blue, green.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
