# Touch in Area

Detect touchscreen touches inside a defined region on Soldered Inkplate 6 Flick.

## Overview

Demonstrates touch detection within a rectangular area using the Inkplate 6 Flick touchscreen. A filled rectangle is drawn on the display; when the user touches inside the rectangle, it moves diagonally. Partial updates are used for fast redraws, with a full refresh when the rectangle resets.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- A black rectangle appears on screen.
- Touching inside the rectangle moves it diagonally across the display.
- When it reaches the lower area, the rectangle resets to the start position.

## Notes

- Touch detection uses `touchscreen.touchInArea(x, y, w, h)`.
- Touchscreen is initialized with `touchscreen.init(true)`.
- Partial updates are used for fast movement; full refresh occurs on reset.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
