# HTTP POST Request

Send HTTP POST requests to a webhook from Soldered Inkplate 6Color.

## Overview

Connects Inkplate 6Color to WiFi and sends HTTP POST requests to webhook.site at a configurable interval. The payload is sent in URL-encoded format. Request status is logged to the serial monitor and displayed on screen.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- Stable WiFi connection

## Setup

### 1. Get a webhook URL

Visit [webhook.site](https://webhook.site) and copy your unique URL. Paste only the path portion (e.g. `/abcd-1234-efgh`) into `WEBHOOK_PATH` in `main.cpp`.

Adjust `POSTING_INTERVAL_MS` to set the posting frequency.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6Color**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- "HTTP POST example" shown on the display.
- Serial monitor logs WiFi connection and POST status.
- webhook.site receives POST requests at the configured interval.

## Notes

- Uses HTTP (port 80); no TLS certificate required.
- Data sent in `application/x-www-form-urlencoded` format.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
