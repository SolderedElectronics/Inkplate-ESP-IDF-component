# Touch in Area

Touch-in-area detection example for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates the `touchInArea()` function by drawing a filled black rectangle on screen that moves diagonally each time it is touched. Partial updates keep repositioning fast. When the rectangle goes off the bottom of the screen it resets to the top-left.

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

## Expected Output

A black rectangle that moves 100 px diagonally each time it is touched. Wraps back to the start position when it goes off-screen.

## Notes

- `touchInArea(x, y, w, h)` returns true when a finger is detected inside the defined region.
- A full refresh is triggered when the rectangle wraps back to the starting position.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
