# Hello World

Display "Hello World!" on Soldered Inkplate 13SPECTRA.

## Overview

Initializes Inkplate 13SPECTRA and draws "Hello World!" text on the e-paper display in 1-bit black and white mode, then calls `display()` to push the framebuffer to the screen.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"Hello World!" displayed on the Inkplate 13SPECTRA e-paper screen.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
