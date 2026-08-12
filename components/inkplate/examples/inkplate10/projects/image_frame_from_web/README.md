# Image Frame From Web

Digital picture frame example for Soldered Inkplate 10: downloads and displays an image from the web, then deep sleeps between refreshes.

## Overview

Connects Inkplate 10 to WiFi, downloads a JPEG image from a configured URL, and renders it full-screen on the e-paper display in 3-bit grayscale. After displaying the image, the ESP32 sets a wake-up timer and enters deep sleep. When the timer expires, the ESP32 restarts and repeats the process, creating a periodically refreshing image frame.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- Stable WiFi connection (2.4 GHz), internet access

## Setup

### 1. Set the image URL

In `main.cpp`, `IMAGE_URL` defaults to a public test endpoint (`loremflickr.com`) that redirects to a random 1200x825 photo on every request. Change it to point to your own image if desired.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate10**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- E-paper: A JPEG image rendered full-screen on the display in 3-bit grayscale.
- Serial: WiFi join status and image draw result, then the device enters deep sleep.

## Notes

- Display mode is 3-bit grayscale (`GRAYSCALE`). Partial update is not available in grayscale mode; this example uses a full refresh via `display.display()`.
- Deep sleep restarts the ESP32; `app_main()` runs again on every wake-up. The refresh interval is configured via `REFRESH_INTERVAL_MIN` (default: 15 minutes).
- HTTPS certificate validation is disabled for this example via `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` (`sdkconfig.defaults`). This is fine for demo/testing only; for production use, validate TLS properly (e.g. `wifi.setCertificate()` with a matching CA certificate).
- `display.image.draw()` downloads the file with `esp_http_client`, which follows HTTP redirects automatically, so no manual redirect-resolution code is needed even though the default URL redirects to a random image on another host.
- Web images and decoding can be RAM-intensive. Large JPEGs or complex images may fail to decode depending on available memory.
- Network endpoints can change behavior (redirects, user-agent filtering, rate limits). If downloads fail, check the log and try a different image source.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
