# Text With Shadow

Render tri-color text with shadow effects on Inkplate 2, then enter deep sleep.

## Overview

Demonstrates basic text rendering and the `drawTextWithShadow()` function on Inkplate 2. The sketch:

- Prints text in black and red
- Renders a red string with a black shadow using `drawTextWithShadow()`
- Draws a signature line with inverted shadow/text colors
- Draws a red line to demonstrate other drawing primitives

After a single full display refresh, the board enters deep sleep.

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

Display shows "Inkplate 2" in black, then in red, then with a black shadow effect; a small "By soldered.com" line with a shadow; and a red diagonal line. Board enters deep sleep after the refresh.

## Notes

- Deep sleep resets the ESP32, so the program restarts from the beginning on wake/reset.
- This sketch does not configure a wake source.
- Inkplate 2 uses a tri-color palette (black/white/red) in 1-bit display mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
