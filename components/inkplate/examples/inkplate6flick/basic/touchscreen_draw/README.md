# Touchscreen Draw

Draw on the e-paper display using the touchscreen on Soldered Inkplate 6 Flick.

## Overview

Demonstrates using the Inkplate 6 Flick touchscreen to draw directly on the e-paper display. Two drawing modes are available via a compile-time define:

- `DRAW_LINE` — draws a continuous line following the finger.
- `DRAW_CIRCLE` — draws filled circles at touch points.

The display is refreshed using partial updates for faster drawing responsiveness.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

In `main.cpp`, enable one of the drawing modes:
```cpp
#define DRAW_LINE
// or
#define DRAW_CIRCLE
```

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- In line mode: a continuous line follows the finger across the display.
- In circle mode: filled circles are drawn wherever the screen is touched.

## Notes

- Only the first detected touch point is used.
- Partial updates refresh only the modified area for faster interaction.
- Touch coordinates are automatically adjusted based on display rotation.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
