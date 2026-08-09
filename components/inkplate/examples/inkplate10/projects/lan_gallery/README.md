# LAN Gallery

Turn Soldered Inkplate 10 into a shared, network-connected picture frame: upload images from any browser on the same LAN and watch them rotate on the e-paper display.

## Overview

Inkplate 10 connects to a WiFi network and starts an HTTP server (using ESP-IDF's native `esp_http_server` component). The server serves a small upload page where anyone on the same network can pick a picture; the browser resizes/re-encodes it to fit the panel resolution and uploads it to the device. The device writes the picture to the microSD card, then periodically scans the card's root directory, picks a random picture, and renders it on the e-paper display. Pictures rotate automatically on a timer, and immediately after every upload.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- MicroSD card (FAT32 formatted)
- WiFi network
- Phone or PC with a web browser on the same network

## Setup

### 1. Prepare the SD card

Format a microSD card as FAT32 and insert it into Inkplate 10. It can start out empty — pictures uploaded through the web page are added to it automatically. You can also copy some BMP/JPG/JPEG pictures onto the card root beforehand if you want the gallery to have content immediately at first boot.

### 2. Select the board and configure WiFi

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate10**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

After flashing, the device connects to the configured WiFi network and prints its IP address to the serial log, for example:

```
I (2345) LAN_GALLERY: Connected! Open http://192.168.1.42/ in your browser
```

Open `http://<printed-ip>/` in a web browser on a device connected to the same network. Pick an image file and press **Upload** — the picture is written to the SD card, and after the upload finishes the display picks a random picture (which may be the one you just uploaded) and shows it, centered, with a small footer label. Pictures also rotate automatically every 30 seconds (`IMAGE_CHANGE_INTERVAL_MS` in `main/main.cpp`).

## Notes

- Arduino's `ESPAsyncWebServer` has no equivalent in this ESP-IDF component. This example is built directly on ESP-IDF's native `esp_http_server` component (`REQUIRES "esp_http_server"` in `main/CMakeLists.txt`) instead, with routes registered via `httpd_register_uri_handler()`.
- The browser posts the picked image as `multipart/form-data`, always re-encoded to JPEG first. Rather than implementing a full multipart parser, the upload handler (`main/webserver.cpp`) scans the raw POST body for the JPEG start-of-image (`0xFF 0xD8`) and end-of-image (`0xFF 0xD9`) markers to locate the picture bytes before writing them to the SD card under a freshly generated file name.
- The uploaded picture is buffered fully in RAM (from SPIRAM when available) for the duration of the request, sized from the request's `Content-Length` header.
- The upload page (`main/html.h`) scales the picture to fit the Inkplate 10 panel resolution (1200x825) client-side before uploading, so JPEGs stay small.
- SD card access is shared between the HTTP upload handler and the picture-rotation loop in `main/main.cpp`; a FreeRTOS mutex serializes access between the two.
- Only files in the SD card root directory are scanned for the gallery; hidden files and files without a `.bmp`/`.jpg`/`.jpeg` extension are ignored.
- Picture dimensions are read directly from the BMP/JPEG file header (without decoding) so pictures can be centered on screen; if this fails, the picture is drawn at (0, 0) instead.
- This example uses the display's grayscale (3-bit) mode; partial update is not available in grayscale mode, so `display.display()` performs a full refresh on every picture change.
- This is a demo web server without authentication; avoid exposing it on untrusted networks.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
