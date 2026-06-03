# Image Converter

Display a custom image on Inkplate 2 using the Soldered Image Converter tool.

## Overview

Demonstrates how to convert any image into a C byte array using the Soldered Image Converter, then display it on Inkplate 2. The tool outputs a header file (`image.h`) which is included directly in the sketch.

## Hardware Required

- Soldered Inkplate 2
- USB cable

## Setup

### 1. Convert your image

1. Open the Image Converter: https://tools.soldered.com/tools/image-converter/
2. Upload your image and select **Inkplate 2** as the target board.
3. Select display mode: **Color (black, white, red)**.
4. Adjust dithering, resize, and other options as needed.
5. Download the generated `image.h` file.

### 2. Add the header file

Place `image.h` in the `main/` folder alongside `main.cpp`.

### 3. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Your converted image displayed on the Inkplate 2 e-paper screen.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
