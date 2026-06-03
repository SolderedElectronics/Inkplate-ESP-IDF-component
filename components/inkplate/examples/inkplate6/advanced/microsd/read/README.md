# MicroSD — Read

Read a text file from a microSD card on Soldered Inkplate 6.

## Overview

Mounts a FAT-formatted microSD card and reads the contents of a text file, displaying the data on the Inkplate 6 e-paper screen. Demonstrates basic file I/O using the ESP-IDF FATFS driver.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- MicroSD card (FAT32 formatted) with a text file

## Setup

### 1. Prepare the SD card

Place a text file (e.g., `text.txt`) in the root of the FAT32-formatted microSD card.

### 2. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Contents of the text file read from microSD and displayed on the Inkplate 6 e-paper screen.

## Notes

- Insert the microSD card before powering on.
- File path in the example must match the actual filename on the card.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
