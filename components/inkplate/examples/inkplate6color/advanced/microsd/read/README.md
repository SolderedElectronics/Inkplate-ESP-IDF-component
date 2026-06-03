# MicroSD Read

Read a text file from a microSD card on Soldered Inkplate 6Color.

## Overview

Demonstrates how to open and read a text file from a FAT-formatted microSD card and display its contents on the Inkplate 6Color e-paper screen.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- MicroSD card (FAT32 formatted) with a text file

## Setup

1. Create a text file (e.g. `text.txt`) on a FAT32-formatted microSD card.
2. Insert the microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Contents of the text file displayed on the Inkplate 6Color screen.

## Notes

- MicroSD card must be FAT or FAT32 formatted.
- Long text may be truncated if it exceeds display capacity.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
