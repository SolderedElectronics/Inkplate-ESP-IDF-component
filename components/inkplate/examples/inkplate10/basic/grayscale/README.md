# Grayscale

Grayscale drawing demo on Soldered Inkplate 10.

## Overview

Demonstrates 3-bit grayscale (8 shades) drawing using Adafruit GFX-compatible functions. Draws shapes and text across the full grayscale range from black to white.

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

Shapes and text rendered in 8 grayscale shades displayed on the e-paper screen.

## Notes

- Grayscale mode uses 3 bits per pixel (shades 0–7: black to white).
- Full display refresh is required in grayscale mode; partial update is not supported.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
