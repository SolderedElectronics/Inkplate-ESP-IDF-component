# Image Frame Gesture

Gesture-controlled image frame slideshow for Soldered Inkplate 4TEMPERA.

## Overview

Implements an image-frame slideshow on Inkplate 4TEMPERA. Images are loaded from a folder on a FAT-formatted microSD card and rendered full-screen in 3-bit grayscale mode.

Navigation is controlled by the onboard APDS9960 gesture sensor: a **LEFT** swipe advances to the next image and a **RIGHT** swipe goes back to the previous one. The gesture sensor is polled continuously in the main loop — this example does not use interrupts or deep sleep, to stay consistent with the rest of the ported examples for this board.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- microSD card (FAT/FAT32 formatted) with image files in a folder

## Setup

### 1. Prepare the SD card

Format a microSD card as FAT/FAT32, create a folder (e.g. `images/`) and copy your images into it. Insert the card into the Inkplate.

### 2. Configure the image folder

Edit `main/main.cpp` and set `IMAGE_FOLDER` to your folder name (path is relative to the SD card root and must end with `/`):

```cpp
#define IMAGE_FOLDER "images/"
```

### 3. Configure the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

- The first supported image found in the folder is displayed on boot.
- Swipe **LEFT** over the APDS9960 sensor area to advance to the next image.
- Swipe **RIGHT** to go back to the previous image.
- Navigation wraps around at either end of the list.

## Expected Output

One image displayed full-screen on the e-paper panel. LEFT/RIGHT gestures change the displayed image with a full-screen refresh.

## Notes

- Display mode is 3-bit grayscale (8 levels). Partial update is not available in grayscale mode, so every image change uses a full e-paper refresh.
- Supported image types depend on the Inkplate image decoder (BMP, JPEG, PNG). Files are matched by extension; anything else in the folder is ignored, along with hidden files.
- Maximum of 512 images per folder (static index array size).
- Gesture sensitivity is set to the lowest gain to reduce accidental triggers.
- This example uses polling (not interrupts) to read gestures, matching the approach used in the APDS9960 sensor example for this board. The original Arduino example used deep sleep with an interrupt-driven wake chain (APDS9960 → IO expander → ESP32 GPIO) for battery-friendly operation; this port keeps the device fully awake and polls instead, for consistency with the rest of the ported examples.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
