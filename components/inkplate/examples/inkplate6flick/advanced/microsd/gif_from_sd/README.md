# GIF From SD

Play a GIF animation from a microSD card on Soldered Inkplate 6 Flick.

## Overview

Loads a GIF file from a FAT-formatted microSD card and plays it back on the Inkplate 6 Flick e-paper screen using partial updates.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- MicroSD card (FAT32 formatted) with a GIF file

## Setup

1. Copy the `cat_gif.gif` file bundled with this example (in `main/`) to the root of a FAT32-formatted microSD card.
2. Insert the microSD card into the Inkplate.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

`cat_gif.gif` loops forever on the display, centered on the screen.

## Notes

- Partial update (and therefore GIF playback) only works in BLACK_AND_WHITE display mode.
- e-paper partial refresh takes far longer than a typical GIF frame delay, so actual playback speed is limited by the panel, not by the file.
- MicroSD card must be FAT or FAT32 formatted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
