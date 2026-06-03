# Set VCOM

Set and verify VCOM voltage for Soldered Inkplate 10.

## Overview

Programs the e-paper panel VCOM voltage to the value defined by `VCOM_VALUE` and displays a grayscale test image for visual verification. VCOM affects image contrast and uniformity; the correct value is printed on the e-paper panel's FPC cable.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

In `main.cpp`, set `VCOM_VALUE` to the value printed on your panel's FPC cable (e.g. `-2.6`).

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"Stored VCOM: X.XX V" and grayscale test bars displayed on screen.

## Notes

- VCOM is stored in non-volatile memory and only needs to be set once.
- An incorrect VCOM value can cause poor contrast or uneven display updates.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
