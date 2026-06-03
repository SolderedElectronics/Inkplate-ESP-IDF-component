# MicroSD Pictures

Display images from a microSD card on Soldered Inkplate 6Color.

## Overview

Reads BMP or JPG images from a FAT-formatted microSD card and displays them on the Inkplate 6Color e-paper screen.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- MicroSD card (FAT32 formatted) with image files

## Setup

1. Copy BMP or JPG image files to the root of a FAT32-formatted microSD card.
2. Insert the microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Images from the microSD card displayed sequentially on the Inkplate 6Color screen.

## Notes

- Supported formats: Windows BMP (1/4/8/24-bit), JPG.
- Images larger than 600×448 px may not render correctly.
- MicroSD card must be FAT or FAT32 formatted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
