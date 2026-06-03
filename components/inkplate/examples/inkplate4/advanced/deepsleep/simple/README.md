# Deep Sleep Slideshow

Low-power image slideshow using ESP32 deep sleep on Inkplate 4TEMPERA.

## Overview

Demonstrates low-power operation by cycling through images using ESP32 timer-based deep sleep. On each wake-up the board draws the next image, performs a full display refresh, and returns to deep sleep. The slideshow loops through 3 images.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

### 1. Prepare image header files

Convert 3 images (600×600 px) using the Image Converter and save them as `picture1.h`, `picture2.h`, `picture3.h` in the `main/` folder.

Image Converter: https://tools.soldered.com/tools/image-converter/

### 2. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A new image is shown on the display every 20 seconds. The slideshow loops through all 3 images indefinitely.

## Notes

- Deep sleep resets the ESP32 on every wake-up; RAM contents are lost.
- Standard partial updates cannot be used across deep sleep cycles.
- This example uses 3-bit (grayscale) mode with full refresh updates.
- Frontlight and touchscreen are disabled before sleep to save power.
- Wake interval is configured via `TIME_TO_SLEEP_US` (default: 20 seconds).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
