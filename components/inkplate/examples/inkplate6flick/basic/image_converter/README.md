# Image Converter

Display a converted image on Soldered Inkplate 6 Flick.

## Overview

Shows how to display an image that has been pre-converted to a C array using the Soldered image converter tool. The image array is included directly in the firmware and rendered onto the e-paper display at startup.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

### 1. Convert your image

Use the [Soldered image converter](https://tools.soldered.com/tools/image-converter/) to convert a BMP/PNG to a C array. Select **Inkplate 6 Flick** as the target board (1024×758 px).

### 2. Add the image array

Place the generated header file in the `main/` folder and include it in `main.cpp`.

### 3. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The converted image displayed on the Inkplate 6 Flick e-paper screen.

## Notes

- Images must be sized to fit the display (1024×758 px) or smaller.
- Larger images will be cropped or may not display correctly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
