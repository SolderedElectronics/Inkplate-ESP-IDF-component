# HTTPS with Certificate

Download images over HTTPS using certificate pinning on Soldered Inkplate 10.

## Overview

Demonstrates secure HTTPS image download with certificate validation. The example:

1. Connects to WiFi and applies a PEM certificate (trust anchor).
2. Downloads and renders a BMP image from a matching host — succeeds.
3. Attempts to download from a different host where the certificate is invalid — fails, showing an error on screen.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- Stable WiFi connection

## Setup

### 1. Provide a PEM certificate

Add your server's PEM certificate to the sketch. The certificate must match the target host.

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

- First HTTPS download succeeds and the BMP image is displayed.
- Second download fails due to certificate mismatch; an error message is shown.

## Notes

- The certificate must match the target host; it cannot be reused for unrelated domains.
- Supported BMP formats: Windows BMP, 1/4/8/24-bit, no compression.
- Update the PEM if the server rotates its certificate.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
