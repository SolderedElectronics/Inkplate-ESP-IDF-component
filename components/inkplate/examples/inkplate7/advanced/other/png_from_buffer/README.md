# PNG from Buffer

Decode and display a PNG image from a memory buffer on Soldered Inkplate 7.

## Overview

Demonstrates decoding a PNG image stored as a byte array in flash memory and rendering it directly on the Inkplate 7 e-paper display, without requiring a microSD card or network connection.

## Hardware Required

- Soldered Inkplate 7
- USB cable

## Setup

### 1. Prepare the PNG

Convert your PNG to a C byte array using the [Soldered image converter](https://tools.soldered.com/tools/image-converter/) and include it in `main.cpp`.

### 2. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate7**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The embedded PNG image decoded and displayed on the Inkplate 7 e-paper screen.

## Notes

- Large PNG images consume significant flash and RAM; keep images reasonably sized.
- Images must fit the display resolution (800×480 px) or smaller.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
