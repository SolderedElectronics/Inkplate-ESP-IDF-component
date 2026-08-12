# Gallery

SD card image gallery with deep sleep on Soldered Inkplate 6COLOR.

## Overview

Initializes the microSD card, scans the root directory for image files (BMP, JPG, PNG), picks one at random, displays it on the 7-color e-paper screen, then enters deep sleep for 5 minutes before waking and repeating. Acts as a low-power digital picture frame.

## Hardware Required

- Soldered Inkplate 6COLOR
- USB cable
- MicroSD card (FAT32 formatted) with BMP, JPG, or PNG images

## Setup

### 1. Prepare the SD card

Copy BMP, JPG, or PNG images to the root of a FAT32-formatted microSD card. Images should be sized for (or smaller than) the 600x448 panel; images larger than 600x448 px may not render correctly.

### 2. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6COLOR**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- A randomly selected image from the SD card displayed on the 600x448 color screen.
- Board enters deep sleep for 5 minutes, then wakes and picks a new image.

## Notes

- Inkplate 6COLOR renders images using its 7-color palette (black, white, green, blue, red, yellow, orange). The palette is selected automatically at compile time based on the board, so no application code changes are needed to use the extra orange color compared to boards with fewer palette entries.
- Deep sleep resets normal RAM; the whole program runs in `app_main` each wakeup.
- Hidden files and files without a supported extension are ignored.
- Adjust `TIME_TO_SLEEP_US` in `main.cpp` to change the refresh interval.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
