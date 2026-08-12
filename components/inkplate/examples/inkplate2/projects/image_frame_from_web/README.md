# Image Frame From Web

Random web image frame with periodic deep sleep refresh for Soldered Inkplate 2.

## Overview

Turns Inkplate 2 into a simple digital picture frame. On every boot, the board connects to WiFi and downloads a randomly generated image from LoremFlickr sized to the Inkplate 2 resolution (212x104), then renders it on the e-paper display with a full refresh. The device then enters deep sleep for `SECS_BETWEEN_IMAGES` seconds. Because deep sleep resets the ESP32, the program always restarts from `app_main()` on every wake cycle and fetches a new random image — there is no explicit refresh loop.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- Stable WiFi connection

## Setup

### 1. Set the image source and refresh interval (optional)

In `main.cpp`, `IMAGE_URL` defaults to a public random-image test endpoint (LoremFlickr) already sized to the Inkplate 2 resolution. Replace it with your own image URL/service if desired. Adjust `SECS_BETWEEN_IMAGES` to change how often the frame refreshes.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate2**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A randomly selected image is downloaded and displayed full-screen (212x104) on every wake cycle. The device then deep sleeps for `SECS_BETWEEN_IMAGES` seconds before repeating.

## Notes

- `display.image.draw()` downloads over HTTP/HTTPS and follows redirects internally, so no manual redirect-resolution logic is needed to reach the final image URL.
- This example uses 1-bit (black & white) display mode.
- Deep sleep restarts the ESP32 on every wake-up; all logic lives in `app_main()`, and RAM contents are lost between cycles.
- Web/API behavior can change: if the image provider changes its response format, requests may start failing.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
