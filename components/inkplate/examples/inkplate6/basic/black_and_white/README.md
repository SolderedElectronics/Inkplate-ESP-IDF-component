# Black and White

Draw basic shapes and text in 1-bit black and white mode on Soldered Inkplate 6.

## Overview

Sets Inkplate 6 to 1-bit (BW) display mode and demonstrates drawing primitives: lines, rectangles, circles, triangles, and text. All content is rendered in pure black and white with no grayscale.

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

Various black and white shapes and text rendered on the Inkplate 6 e-paper display (1200×825 px).

## Notes

- 1-bit mode enables partial updates and faster refresh compared to grayscale mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
