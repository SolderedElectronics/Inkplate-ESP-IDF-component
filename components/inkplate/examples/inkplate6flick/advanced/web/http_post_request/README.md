# HTTP POST Request

Send an HTTP POST request from Soldered Inkplate 6 Flick.

## Overview

Connects Inkplate 6 Flick to WiFi and sends an HTTP POST request with a JSON or form-encoded body to a configured endpoint. The server response is displayed on the e-paper screen.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- Stable WiFi connection

## Setup

In `main.cpp`, set the target URL and POST body/headers as required by your server.

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6 Flick**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Server response to the POST request displayed on the Inkplate 6 Flick e-paper screen.

## Notes

- For HTTPS POST, see the `https_post_request` example.
- Content-Type header must match the body format expected by the server.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
