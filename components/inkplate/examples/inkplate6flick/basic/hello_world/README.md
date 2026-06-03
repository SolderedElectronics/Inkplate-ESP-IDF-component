# Hello World

Display "Hello World!" on Soldered Inkplate 6 Flick.

## Overview

Initializes Inkplate 6 Flick and draws "Hello World!" text on the e-paper display in 1-bit black and white mode, then calls `display()` to push the framebuffer to the screen.

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

"Hello World!" displayed on the Inkplate 6 Flick e-paper screen.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
