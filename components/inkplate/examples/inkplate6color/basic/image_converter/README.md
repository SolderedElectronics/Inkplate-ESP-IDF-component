# Image Converter

Display a converted color image on Soldered Inkplate 6Color.

## Overview

Demonstrates how to display an image converted using the Soldered image converter tool. The converted image data is embedded as a C header file and drawn directly from flash memory.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

1. Use the [Soldered image converter](https://tools.soldered.com/tools/image-converter/) to convert your image to a C header compatible with Inkplate 6Color.
2. Replace the image header in the `main/` folder with your converted file.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The converted image displayed on the Inkplate 6Color e-paper screen.

## Notes

- Target image size: 600×448 px for full-screen display.
- The converter maps image colors to the 7 supported e-paper colors.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
