# Frontlight Control

Frontlight brightness fade example for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates control of the built-in frontlight by gradually fading brightness up from 0 to 63 and back down to 0, repeating the cycle four times. After the cycles complete, the frontlight is held at a fixed brightness value.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The frontlight visibly fades in and out four times, then holds steady at brightness 31.

## Notes

- Brightness range: 0 (off) to 63 (maximum).
- `frontlight.setState(true)` must be called before `setBrightness()`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
