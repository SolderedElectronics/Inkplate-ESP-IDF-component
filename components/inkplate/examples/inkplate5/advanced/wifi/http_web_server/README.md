# WiFi Web Server

Use Inkplate 5 as a standalone WiFi access point and HTTP web server to send text to the display from a browser.

## Overview

Inkplate 5 creates its own WiFi access point and runs an HTTP web server. Any WiFi-capable device can connect and open a web page to type and send text directly to the e-paper display.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- WiFi-capable device with a web browser (PC, laptop, smartphone)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

1. Flash and power on Inkplate 5.
2. Connect your device to the Inkplate WiFi access point.
3. Open the IP address shown on the Inkplate display in a web browser.
4. Enter text into the web page and press **Send to display**.
5. The submitted text appears on the Inkplate display.

## Expected Output

- Inkplate display shows its IP address and the last received text.
- The web page allows sending custom text to the display.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
