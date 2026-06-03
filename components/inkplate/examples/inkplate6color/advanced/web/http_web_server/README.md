# HTTP Web Server

Run a web server on Soldered Inkplate 6Color to display text from a browser.

## Overview

Starts an HTTP web server on Inkplate 6Color. After connecting to the same network, open the Inkplate's IP address in a browser, enter text into the web page, and press "Send to display" — the text appears on the e-paper screen.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- Stable WiFi connection

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6Color**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows its IP address after connecting to WiFi.
- Text submitted via the web page appears on the display.

## Notes

- Device sending requests must be on the same WiFi network as the Inkplate.
- Intended for simple prototyping; more advanced web logic can be built on top.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
