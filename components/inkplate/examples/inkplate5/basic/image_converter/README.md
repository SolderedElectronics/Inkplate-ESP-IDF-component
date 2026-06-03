# Image Converter

Display a pre-converted image on Soldered Inkplate 5.

## Overview

Shows how to display an image converted to a C header file using the Soldered online image converter tool. Image data is stored in `image_ex.h` and drawn directly from ESP32 flash.

## Hardware Required

- Soldered Inkplate 5
- USB cable

## Setup

### 1. Convert your image

1. Open the Image Converter: https://tools.soldered.com/tools/image-converter/
2. Upload your image and select **Inkplate 5** as the target board.
3. Download the generated header file and save it as `image_ex.h` in the `main/` folder.

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The converted image displayed at position (0, 0) on the e-paper screen.

## Notes

- `image_w` and `image_h` are defined in the generated `image_ex.h`.
- For grayscale mode, change `BLACK_AND_WHITE` to `GRAYSCALE` and remove the `BLACK` color argument from `image.draw()`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
