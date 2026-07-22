# HTTP Web Server

Host a simple web server on Soldered Inkplate 7.

## Overview

Connects Inkplate 7 to WiFi and starts a lightweight HTTP server. A browser on the same network can send commands or content to Inkplate 7, which are then displayed on the e-paper screen. The board's IP address is shown on the display after connecting.

## Hardware Required

- Soldered Inkplate 7
- USB cable
- Stable WiFi connection
- Computer or phone on the same network

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate7**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate 7 displays its IP address after connecting to WiFi.
- Open that IP in a browser to interact with the board via the web interface.

## Notes

- Inkplate 7 and the client device must be on the same WiFi network.
- The server handles one request at a time.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
