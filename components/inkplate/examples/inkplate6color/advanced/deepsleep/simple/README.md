# Simple Deep Sleep

Image slideshow using deep sleep on Soldered Inkplate 6Color.

## Overview

Displays a sequence of three embedded color images as a slideshow, entering deep sleep between each image. The slide index is stored in RTC memory (`RTC_DATA_ATTR`) to advance to the next image on each wake-up.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Images displayed one by one, with the board waking from deep sleep to advance the slideshow.

## Notes

- Deep sleep resets normal RAM; only RTC-backed variables (`RTC_DATA_ATTR`) survive across cycles.
- Color e-paper requires a full refresh; partial update is not supported.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
