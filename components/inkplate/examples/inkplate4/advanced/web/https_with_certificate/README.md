# HTTPS with Certificate

Securely download and display a BMP image over HTTPS using certificate pinning on Inkplate 4TEMPERA.

## Overview

Demonstrates secure HTTPS image download by providing a trusted PEM certificate for server validation. The example:

1. Connects to WiFi and applies a PEM certificate (trust anchor).
2. Downloads and renders a BMP image from a host that matches the certificate — succeeds.
3. Attempts to download from a different host where the certificate is not valid — fails, showing an error on screen.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- Stable WiFi connection

## Setup

### 1. Provide a PEM certificate

Add your server's PEM certificate to the sketch. The certificate must match the target host.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate4**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- First HTTPS image download succeeds and the image is displayed.
- Second download fails due to certificate mismatch; an error message is shown on the display.

## Notes

- Certificate must match the target host; it cannot be reused for unrelated domains.
- Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth, no compression.
- If the server rotates its certificate, update the PEM in the sketch.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
