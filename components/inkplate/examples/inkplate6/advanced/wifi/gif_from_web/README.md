# GIF From Web

Download and play a GIF animation from the web on Soldered Inkplate 6.

## Overview

Connects Inkplate 6 to WiFi, downloads a GIF file from a configured URL, and plays it back on the e-paper display using partial updates.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- Stable WiFi connection

## Setup

### 1. Set the GIF URL

In `main.cpp`, set `GIF_URL` to a direct link to a GIF file (no HTML redirect pages).

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The GIF from `GIF_URL` plays on the display, centered on the screen, looping until reset/power-cycled.

## Notes

- Partial update (and therefore GIF playback) only works in BLACK_AND_WHITE display mode.
- The whole GIF file is held in memory at once - make sure the file is small enough to fit.
- e-paper partial refresh takes far longer than a typical GIF frame delay, so actual playback speed is limited by the panel, not by the file.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
