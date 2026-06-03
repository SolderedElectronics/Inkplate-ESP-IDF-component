# PNG from Buffer

Display a PNG image loaded into a RAM buffer on Soldered Inkplate 10.

## Overview

Demonstrates how to read a PNG file from a microSD card into a RAM buffer and display it using `drawPngFromBuffer()`. The same technique applies to PNG data received from any source — a network socket, a serial transfer, or a flash partition.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- MicroSD card with a file named `image.png`

## Setup

1. Copy a PNG file named `image.png` to the root of a FAT32-formatted microSD card.
2. Insert the microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The PNG image loaded from the SD card into RAM is displayed on the Inkplate 10 screen.

## Notes

- The PNG is fully loaded into RAM before rendering; ensure the image fits in available heap.
- Images larger than 1200×825 px may not render correctly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
