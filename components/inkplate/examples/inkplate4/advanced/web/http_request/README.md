# WiFi HTTP Request

Fetch content from the web over HTTP and display it on Inkplate 4TEMPERA.

## Overview

Connects to WiFi, performs a basic HTTP GET request to retrieve data from the Internet, and prints the raw response body on the e-paper display. No HTML parsing is performed.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- Stable WiFi connection

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate4**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Inkplate display shows the raw text/HTML content fetched from the configured web URL.

## Notes

- This example demonstrates basic HTTP communication only; no HTML parsing is performed.
- Large responses may require text size adjustments to fit the display.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
