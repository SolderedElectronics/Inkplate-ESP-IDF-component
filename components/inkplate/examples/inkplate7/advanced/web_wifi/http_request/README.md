# HTTP Request

Fetch data from a web server over HTTP on Soldered Inkplate 7.

## Overview

Connects Inkplate 7 to WiFi, sends an HTTP GET request to a configured URL, and displays the response on the e-paper screen. Demonstrates basic HTTP client usage with the ESP-IDF HTTP client library.

## Hardware Required

- Soldered Inkplate 7
- USB cable
- Stable WiFi connection

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate7**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

HTTP response body (or a portion of it) displayed on the Inkplate 7 e-paper screen.

## Notes

- Only plain HTTP is used; for HTTPS see the `https_with_certificate` example.
- Long responses may be truncated to fit the display.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
