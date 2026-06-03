# MicroSD — Pictures

Display BMP images from a microSD card on Soldered Inkplate 6.

## Overview

Reads BMP image files from a FAT-formatted microSD card and displays them on the Inkplate 6 e-paper display. The example iterates through images stored on the card, rendering each one in sequence.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- MicroSD card (FAT32 formatted)
- BMP image files on the card

## Setup

### 1. Prepare the SD card

Format the microSD card as FAT32 and copy compatible BMP images to the root directory.

### 2. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

BMP images from the microSD card displayed sequentially on the Inkplate 6 e-paper screen.

## Notes

- Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth.
- Images must fit the display (1200×825 px); oversized images may be cropped.
- Insert the microSD card before powering on.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
