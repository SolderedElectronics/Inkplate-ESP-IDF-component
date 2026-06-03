# Hello World

Basic "Hello World" display example for Soldered Inkplate 10.

## Overview

Initializes Inkplate 10 and prints "Hello World!" on the e-paper screen using built-in text rendering functions compatible with the Adafruit GFX library.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"Hello World!" displayed on the e-paper screen.

## Notes

- `display.clearDisplay()` clears only the internal framebuffer, not the physical panel.
- `display.display()` must be called to push the framebuffer to the e-paper panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
