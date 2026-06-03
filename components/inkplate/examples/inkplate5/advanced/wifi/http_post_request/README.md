# WiFi HTTP POST Request

Send periodic HTTP POST requests from Inkplate 5 to webhook.site.

## Overview

Connects Inkplate 5 to a WiFi network and sends periodic HTTP POST requests to [webhook.site](https://webhook.site) — a free online service for inspecting HTTP requests in real time.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- Stable WiFi connection

## Setup

### 1. Get a webhook URL

1. Visit https://webhook.site and copy your unique webhook URL.
2. Paste only the path part (e.g. `/abcd-1234-efgh`) into `WEBHOOK_PATH` in `main.cpp`.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate5**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows example information.
- webhook.site displays incoming POST requests every 20 seconds.

## Notes

- Uses HTTP (port 80) for simplicity; not encrypted.
- Data is sent in URL-encoded format (`application/x-www-form-urlencoded`).
- Replace the example payload with real sensor readings as needed.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
