# Touchscreen Draw

Draw on screen with touch for Soldered Inkplate 4TEMPERA.

## Overview

Uses the touchscreen to draw on the e-paper display. Two modes are available (selected via `#define`):

- **DRAW_LINE** (default) — a continuous line is drawn between successive touch positions.
- **DRAW_CIRCLE** — a filled circle is drawn at each touch point.

Partial updates keep the drawing responsive.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

To switch between draw modes, change the `#define` in `main.cpp`:
- `#define DRAW_LINE` for line drawing (default)
- `#define DRAW_CIRCLE` for circle drawing

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Lines (or circles) drawn on the e-paper wherever the screen is touched.

## Notes

- `partialUpdate(false, true)` keeps e-paper power on for faster successive updates.
- The drawing is not cleared automatically; perform a full refresh to reset.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
