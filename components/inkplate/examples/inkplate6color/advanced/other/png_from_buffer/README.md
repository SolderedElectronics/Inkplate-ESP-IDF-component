# PNG from Buffer

Display a PNG image loaded into a RAM buffer on Soldered Inkplate 6Color.

## Overview

Demonstrates how to read a PNG file from a microSD card into a heap buffer and display it using `drawPngFromBuffer()`. The same technique applies to PNG data received from any source — a network socket, a serial transfer, or a flash partition.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- MicroSD card with a PNG image file

## Setup

1. Copy a PNG file to the root of a FAT32-formatted microSD card.
2. Insert the microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The PNG image loaded into RAM is displayed on the Inkplate 6Color screen.

## Notes

- The entire PNG file is loaded into heap memory before decoding. Ensure the file fits in available RAM (~300 KB free on ESP32).
- Dithering is enabled by default; pass `false` as the fifth argument to `drawPngFromBuffer()` to disable it.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
