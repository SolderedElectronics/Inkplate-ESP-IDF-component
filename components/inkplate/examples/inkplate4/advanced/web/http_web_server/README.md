# WiFi Web Server

Use Inkplate 4TEMPERA as a standalone WiFi access point and HTTP web server to send text to the display from a browser.

## Overview

Inkplate 4TEMPERA creates its own WiFi access point (SSID: "Inkplate 4TEMPERA") and runs an HTTP web server. Any WiFi-capable device can connect and open a web page to type and send text directly to the e-paper display.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- WiFi-capable device with a web browser (PC, laptop, smartphone)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

1. Flash and power on Inkplate 4TEMPERA.
2. Connect your device to the **"Inkplate 4TEMPERA"** WiFi network (password: `Soldered`).
3. Open the IP address shown on the Inkplate display in a web browser.
4. Enter text into the web page and press **Send to display**.
5. The submitted text appears on the Inkplate display.

## Expected Output

- Inkplate display shows its IP address and the last received text.
- The web page allows sending custom text to the display.

## Notes

- Intended for prototyping; more advanced web interfaces can be built on this base.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
