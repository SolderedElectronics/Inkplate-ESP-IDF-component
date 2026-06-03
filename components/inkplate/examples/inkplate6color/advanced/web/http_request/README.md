# HTTP Request

Fetch and display web content over HTTP on Soldered Inkplate 6Color.

## Overview

Connects Inkplate 6Color to WiFi and fetches raw content from a remote web server using HTTP. The received content is printed on the e-paper display.

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

Raw text or HTML fetched from the web server displayed on the Inkplate 6Color screen.

## Notes

- No HTML parsing or content extraction is performed.
- Large responses may require text size adjustments to fit the display.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
