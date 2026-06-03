# Show Pictures from Web

Download and display an image from the web on Soldered Inkplate 13SPECTRA.

## Overview

Connects Inkplate 13SPECTRA to WiFi, downloads an image from a configured URL, and renders it on the e-paper display using the Inkplate image drawing functions.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable WiFi connection

## Setup

### 1. Set the image URL

In `main.cpp`, set the image URL to point directly to a compatible image file.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The image downloaded from the web is displayed on the Inkplate 13SPECTRA e-paper screen.

## Notes

- Supported formats: BMP (1/4/8/24-bit), JPG, PNG.
- Images must fit the display (1600×1200 px); large images may not render correctly.
- The URL must point directly to the image file (no HTML redirect pages).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
