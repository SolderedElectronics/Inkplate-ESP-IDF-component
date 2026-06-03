# Simple Deep Sleep

Image slideshow using deep sleep on Soldered Inkplate 10.

## Overview

Displays a sequence of embedded grayscale images as a slideshow, entering deep sleep for 20 seconds between each image. The display index is stored in RTC memory to advance to the next image on each wake-up.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A new image from the embedded slideshow is displayed every 20 seconds.

## Notes

- Deep sleep restarts the program from the beginning on each wake-up; RAM contents are lost.
- This example uses 3-bit grayscale mode, which requires full refresh updates.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
