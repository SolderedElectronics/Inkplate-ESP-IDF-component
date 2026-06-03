# Image Converter

Display a converted bitmap image on Soldered Inkplate 10.

## Overview

Demonstrates how to display a bitmap image converted using the Soldered image converter tool. The converted image data is embedded as a C header file and drawn directly from flash memory.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

1. Use the [Soldered image converter](https://tools.soldered.com/tools/image-converter/) to convert your image to a C header.
2. Replace the image header in the `main/` folder with your converted file.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The converted image displayed on the Inkplate 10 e-paper screen.

## Notes

- Target image size: 1200×825 px for full-screen display.
- Both 1-bit (BW) and 3-bit (grayscale) formats are supported by the converter.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
