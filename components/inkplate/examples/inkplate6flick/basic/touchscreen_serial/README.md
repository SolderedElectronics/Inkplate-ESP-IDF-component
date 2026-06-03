# Touchscreen Serial

Monitor touch events via serial on Soldered Inkplate 6 Flick.

## Overview

Reads touch events from the Inkplate 6 Flick touchscreen and prints detected touch coordinates to the serial monitor. Supports multi-touch — up to two simultaneous fingers. When a touch occurs, the number of detected fingers and their coordinates are printed. When all fingers are released, a "Release" message is printed.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Open a serial terminal at **115200 baud**:
- Touch the screen with one finger → coordinates (X, Y) printed.
- Touch with two fingers simultaneously → both coordinates printed.
- Lift all fingers → "Release" printed.

## Notes

- The touchscreen supports up to two simultaneous touch points.
- Touch coordinates are automatically adjusted if display rotation changes.
- This example sets display rotation to orientation 2.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
