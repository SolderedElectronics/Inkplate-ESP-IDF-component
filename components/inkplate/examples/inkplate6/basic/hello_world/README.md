# Hello World

Display "Hello World!" on Soldered Inkplate 6.

## Overview

Initializes Inkplate 6 and draws "Hello World!" text on the e-paper display, then calls `display()` to push the framebuffer to the screen.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"Hello World!" displayed on the Inkplate 6 e-paper screen.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
