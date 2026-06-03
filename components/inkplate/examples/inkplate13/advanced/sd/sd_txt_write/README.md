# SD Text Write

Write data to a text file on a microSD card from Soldered Inkplate 13SPECTRA.

## Overview

Mounts a FAT-formatted microSD card and writes a text string to a file. Demonstrates basic file write operations using the ESP-IDF FATFS driver. Confirms success by reading the file back and displaying the contents on the e-paper screen.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- MicroSD card (FAT32 formatted)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A text file is created on the microSD card. The written content is read back and displayed on the Inkplate 13SPECTRA e-paper screen for verification.

## Notes

- Insert the microSD card before powering on.
- Writing to a file that already exists will overwrite its content.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
