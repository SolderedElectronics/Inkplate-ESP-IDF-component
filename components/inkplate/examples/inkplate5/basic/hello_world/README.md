# Hello World

Basic "Hello World!" example for Soldered Inkplate 5.

## Overview

Initializes the display and prints "Hello World!" using built-in text rendering compatible with the Adafruit GFX library. A good starting point for any Inkplate 5 project.

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

"Hello World!" displayed on the e-paper screen in black and white mode.

## Notes

- `display.clearDisplay()` clears only the internal framebuffer.
- `display.display()` must be called to push changes to the physical panel.
- This example uses 1-bit (black & white) display mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
