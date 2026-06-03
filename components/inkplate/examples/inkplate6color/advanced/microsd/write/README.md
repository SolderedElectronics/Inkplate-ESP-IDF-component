# MicroSD Write

Write a text file to a microSD card on Soldered Inkplate 6Color.

## Overview

Demonstrates how to create and write a text file to a FAT-formatted microSD card inserted into Inkplate 6Color.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- MicroSD card (FAT32 formatted)

## Setup

Insert a FAT32-formatted microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A text file is created and written to the microSD card. Success or failure is shown on the Inkplate 6Color display.

## Notes

- MicroSD card must be FAT or FAT32 formatted.
- If the file already exists it will be overwritten.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
