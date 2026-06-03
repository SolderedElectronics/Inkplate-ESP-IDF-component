# PNG from Buffer

Display a PNG image loaded into a RAM buffer on Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates how to read a PNG file from an SD card into a RAM buffer and then display it using `drawPngFromBuffer()`. The same technique applies to PNG data received from any source — a network socket, a serial transfer, a flash partition, etc.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- microSD card (FAT/FAT32 formatted)

## Setup

### 1. Prepare the SD card

Copy a PNG file named `image.png` to a FAT-formatted microSD card. Insert the card into the Inkplate.

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The PNG image is read from the SD card into RAM and rendered on the e-paper display.

## Notes

- The entire PNG file is loaded into heap memory before decoding — ensure the file fits in available RAM.
- Dithering is enabled by default; pass `false` as the fifth argument to `drawPngFromBuffer()` to disable it.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
