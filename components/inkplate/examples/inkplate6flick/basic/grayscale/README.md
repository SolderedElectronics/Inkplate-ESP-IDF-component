# Grayscale

Draw shapes and text in 4-bit grayscale mode on Soldered Inkplate 6 Flick.

## Overview

Sets Inkplate 6 Flick to 4-bit grayscale mode (16 shades) and demonstrates drawing primitives including filled rectangles, circles, and text in varying gray levels.

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

Shapes and text in multiple gray shades rendered across the Inkplate 6 Flick e-paper display (1024×758 px).

## Notes

- 4-bit grayscale mode uses 16 shades (0 = black, 15 = white).
- Grayscale mode requires full refresh; partial update is not available.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
