# MicroSD — Display Pictures

Display images from a microSD card on Soldered Inkplate 5.

## Overview

Demonstrates how to load image files from a FAT-formatted microSD card and display them on the e-paper panel using the Inkplate graphics library.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- microSD card (FAT/FAT32 formatted)

## Setup

### 1. Prepare the SD card

Copy a supported image file (e.g. `coast.jpg`) to a FAT-formatted microSD card. Insert the card into the Inkplate.

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The selected image from the SD card is shown on the Inkplate display.

## Notes

- Supported formats: BMP, JPEG, PNG (with library limitations).
- Supported color depths: 1-bit (BW), 4-bit, 8-bit, and 24-bit.
- Maximum supported resolution: 1280×720 pixels.
- SD card must be FAT/FAT32 formatted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
