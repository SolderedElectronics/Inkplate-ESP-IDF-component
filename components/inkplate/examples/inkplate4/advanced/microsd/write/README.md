# MicroSD — Write File

Write a text file to a microSD card on Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates how to initialize the SD card, create a `.txt` file, write data into it, and properly close the file using the Inkplate SD card interface.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- microSD card (FAT/FAT32 formatted)

## Setup

### 1. Prepare the SD card

Insert a FAT-formatted microSD card into the Inkplate.

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

File `message.txt` created on the SD card with the text defined in the sketch.

## Notes

- SD card must be FAT/FAT32 formatted.
- Always close files after writing to prevent data corruption.
- SD card is put into sleep mode after the operation to reduce power consumption.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
