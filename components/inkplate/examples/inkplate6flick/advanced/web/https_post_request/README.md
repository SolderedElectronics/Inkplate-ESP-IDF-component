# HTTPS POST Request

Send periodic HTTPS POST requests with JSON data from Soldered Inkplate 6 Flick.

## Overview

Connects Inkplate 6 Flick to WiFi and sends HTTPS POST requests with JSON data to the JSONPlaceholder test API every 10 seconds. The HTTP status code and server response are printed to the serial monitor. Certificate verification is disabled for simplicity.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- Stable WiFi connection

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6 Flick**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate 6 Flick display shows "HTTPS POST Request example" and serial monitor hint.
- Serial monitor (115200 baud) shows HTTP status code and JSON response from the server every 10 seconds.

## Notes

- Certificate verification is disabled (`CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY`). For production use, validate the server certificate.
- JSONPlaceholder is a fake REST API for testing; it echoes data but does not persist it.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
