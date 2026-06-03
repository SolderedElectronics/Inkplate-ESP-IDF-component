# HTTPS POST Request

Send periodic HTTPS POST requests with JSON payload from Inkplate 4TEMPERA.

## Overview

Demonstrates how to connect Inkplate 4TEMPERA to WiFi and send periodic HTTPS POST requests with a JSON payload to the JSONPlaceholder fake REST API. The HTTP status code and response body are logged via serial monitor. HTTPS is used without certificate validation (insecure mode — for demo use only).

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- Stable WiFi connection with Internet access

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate4**
- **WiFi Configuration → Enter your SSID and password**
- **Component config → ESP-TLS → Allow potentially insecure options → Skip server certificate verification by default**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows a short message prompting to open the serial monitor.
- Serial monitor shows WiFi connection status, HTTP status code, and the response body.

## Notes

- `skip_cert_common_name_check` disables certificate validation — not for production use. See the `https_with_certificate` example for secure usage.
- JSONPlaceholder is a fake API: responses look real but data is not persisted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
