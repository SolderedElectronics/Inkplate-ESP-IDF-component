# MicroSD — Write File

Write a text file to a microSD card on Soldered Inkplate 5.

## Overview

Demonstrates how to initialize the SD card, create a `.txt` file, write data into it, and properly close the file using the Inkplate SD card interface. Writes a short text string into `test.txt`.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- microSD card (FAT/FAT32 formatted)

## Setup

Insert a FAT-formatted microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

File `test.txt` created on the SD card with the text defined in the sketch.

## Notes

- SD card must be FAT/FAT32 formatted.
- Always close files after writing to prevent data corruption.
- SD card is put into sleep mode after the operation to reduce power consumption.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
