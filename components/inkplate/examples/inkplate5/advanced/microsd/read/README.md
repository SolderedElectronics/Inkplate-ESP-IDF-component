# MicroSD — Read File

Read and display a text file from a microSD card on Soldered Inkplate 5.

## Overview

Demonstrates how to open a `.txt` file from a FAT-formatted microSD card and display its contents on the e-paper display.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- microSD card (FAT/FAT32 formatted)

## Setup

### 1. Prepare the SD card

Create a file named `text.txt` on a FAT-formatted microSD card. Insert the card into the Inkplate.

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The contents of `text.txt` are displayed on the Inkplate e-paper screen.

## Notes

- File name must be exactly `text.txt` for this example.
- SD card must be FAT/FAT32 formatted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
