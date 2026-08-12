# Image Frame From Web

Digital picture frame example for Soldered Inkplate 13SPECTRA: downloads and displays an image from the web, dithered to the panel's 6-color palette, then deep sleeps between refreshes.

## Overview

Connects Inkplate 13SPECTRA to WiFi, downloads a JPEG image from a configured URL, and renders it full-screen on the e-paper display, dithered down to the panel's 6-color palette. After displaying the image, the ESP32 sets a wake-up timer and enters deep sleep. When the timer expires, the ESP32 restarts and repeats the process, creating a periodically refreshing image frame.

The image is downloaded with `display.image.draw(url, x, y, dither, invert)`, whose underlying downloader (`WiFi::downloadFile()`/`downloadFileHTTPS()`) already follows HTTP redirects itself, so no manual "Location" header handling is needed even though the default URL redirects to a random image on another host.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable WiFi connection (2.4 GHz), internet access

## Setup

### 1. Set the image URL

In `main/main.cpp`, `IMAGE_URL` defaults to a public test endpoint (`loremflickr.com`) that redirects to a random 1600x1200 photo on every request. Change it to point to your own image if desired.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13** (used for both Inkplate 13 and Inkplate 13SPECTRA)
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- E-paper: A JPEG image rendered full-screen on the display, dithered to the panel's 6-color palette (black, white, yellow, red, blue, green).
- Serial: WiFi join status and image draw result, then the device enters deep sleep.
- On a download/decode failure: an "Image download error" message is shown on the display instead of the image.

## Notes

- Color image path: Inkplate 13SPECTRA uses `ImageColor` (not the plain grayscale `Image` class), so `display.image.draw()` dithers/quantizes the downloaded JPEG down to the panel's 6 colors: black, white, yellow, red, blue, green. There is **no `INKPLATE_ORANGE`** on this board, unlike Inkplate 6Color's 7-color palette.
- `display` is a local variable inside `app_main()`, not a file-scope global — a file-scope `Inkplate display;` would race the library's own global I2C/PCAL peripheral objects (in `BoardCommon.cpp`), since C++ doesn't guarantee cross-translation-unit static init order.
- Orientation: the board defaults to rotation 3 (landscape) right after construction, so no explicit `setRotation()` call is made here, matching the original sketch.
- Redirect handling: the original `Inkplate13SPECTRA_Image_Frame_From_Web` Arduino sketch manually resolves the image host's redirect (`HTTPClient` + `collectHeaders()`/`setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS)`/`getLocation()`) before downloading. This port drops that manual step: `display.image.draw()`'s downloader already follows up to 5 HTTP redirects internally (see `components/inkplate/src/features/WiFi.cpp`), so passing `IMAGE_URL` straight to `display.image.draw()` is enough.
- Deep sleep restarts the ESP32; `app_main()` runs again on every wake-up. The refresh interval is configured via `REFRESH_INTERVAL_MIN` (default: 15 minutes, matching the original sketch).
- HTTPS certificate validation is disabled via `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` (`sdkconfig.defaults`), for parity with other ported examples. The default `IMAGE_URL` is plain HTTP so this isn't actually exercised; if you point it at an HTTPS host, note this disables server certificate checking — fine for demo/testing only, not for production use.
- WiFi connection failure isn't specially handled: `display.wifi.waitForConnect()` uses its default (10 s) timeout, and if it fails, the subsequent `display.image.draw()` call simply fails too (no network), which shows the on-screen "Image download error" message. Either way, the device still deep sleeps on schedule and retries on the next wake-up cycle.
- Web images and decoding can be RAM-intensive. Large JPEGs or complex images may fail to decode depending on available memory.
- Network endpoints can change behavior (redirects, user-agent filtering, rate limits). If downloads fail, check the log and try a different image source.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
