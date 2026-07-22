# Show JPG with HTTP Client

Download a JPG image into a buffer and display it on Soldered Inkplate 7.

## Overview

Demonstrates downloading a JPG (or PNG) image into a PSRAM buffer using the WiFi download API, then drawing it on the e-paper display from the buffer. This approach is useful when the image data needs to be pre-processed before display.

## Hardware Required

- Soldered Inkplate 7
- USB cable
- Stable WiFi connection

## Setup

### 1. Set the image URL

In `main.cpp`, set `IMAGE_URL` to any accessible JPG or PNG URL.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate7**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

JPG image downloaded into PSRAM and displayed on the Inkplate 7 e-paper screen.

## Notes

- Image is downloaded into PSRAM; ensure the image size fits available memory.
- Certificate verification is disabled (`CONFIG_ESP_TLS_INSECURE=y`).
- Images must fit the display resolution (800×480 px) or smaller.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
