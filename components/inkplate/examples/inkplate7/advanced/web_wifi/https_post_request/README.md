# HTTPS POST Request

Send periodic HTTPS POST requests from Soldered Inkplate 7.

## Overview

Connects Inkplate 7 to WiFi and sends periodic HTTPS POST requests to [webhook.site](https://webhook.site) — a free online service for real-time HTTP request inspection. Useful for testing IoT data transmission over HTTPS.

## Hardware Required

- Soldered Inkplate 7
- USB cable
- Stable WiFi connection

## Setup

### 1. Get a webhook URL

Visit [https://webhook.site](https://webhook.site) and copy your unique webhook URL path (e.g. `/abcd-1234-efgh`).

### 2. Configure the example

In `main.cpp`, paste the path into `WEBHOOK_PATH`.

### 3. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate7**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate 7 display shows example information.
- webhook.site displays incoming POST requests every 20 seconds.

## Notes

- Certificate verification is disabled (`CONFIG_ESP_TLS_INSECURE=y`) for simplicity.
- Replace `WEBHOOK_PATH` with your unique webhook.site path before building.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
