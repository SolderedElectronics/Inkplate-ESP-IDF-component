# Show Picture from Web

Download and display a BMP image from the web on Inkplate 6.

## Overview

Connects Inkplate 6 to WiFi, downloads a BMP image from a configured URL, and renders it on the e-paper display using the Inkplate image drawing functions.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- Stable WiFi connection

## Setup

### 1. Set the image URL

In `main.cpp`, set the image URL to point directly to a compatible BMP file.

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

The BMP image downloaded from the web is displayed on the Inkplate 6 e-paper screen.

## Notes

- Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth.
- Images must fit the display (1200×825 px); large images may not render correctly.
- The URL must point directly to the image file (no HTML redirect pages).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
