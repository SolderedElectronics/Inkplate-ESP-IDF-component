# Image Uploader

Turn Soldered Inkplate 10 into a standalone web app for uploading an image from a phone or PC and displaying it on the e-paper screen.

## Overview

Inkplate 10 connects to a WiFi network and starts an HTTP server (using ESP-IDF's native `esp_http_server` component). The server serves an upload page that lets the user take a photo or choose an image from their gallery. The page scales and crops the image to the panel resolution in the browser, then uploads it to the device. The device decodes the uploaded JPEG and renders it on the e-paper display in grayscale.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- WiFi network
- Phone or PC with a web browser

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate10**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

After flashing, the device connects to the configured WiFi network and prints its IP address to the serial log (and shows it on the e-paper display), for example:

```
I (2345) IMAGE_UPLOADER: Connected! Open http://192.168.1.42/ in your browser
```

Open `http://<printed-ip>/` in a web browser on a device connected to the same network. The upload page lets you take a photo or choose an image from your gallery; pressing **Upload** sends it to the device, which decodes it and renders it on the Inkplate 10 e-paper display.

## Notes

- Arduino's `WebServer` class has no equivalent in this ESP-IDF component. This example is built directly on ESP-IDF's native `esp_http_server` component (`REQUIRES "esp_http_server"` in `main/CMakeLists.txt`) instead, with routes registered via `httpd_register_uri_handler()`.
- The browser posts the picked/captured image as `multipart/form-data`. Rather than implementing a full multipart parser, the upload handler scans the received body for the JPEG start-of-image marker (`0xFF 0xD8`) to locate the embedded image before decoding it.
- The uploaded image is buffered fully in RAM, sized from the request's `Content-Length` header; very large uploads may fail to allocate.
- The upload page (`main/html.h`) scales/crops the picture to the Inkplate 10 panel resolution (1200x825) client-side before uploading, so JPEGs stay small.
- This example uses the display's default grayscale (3-bit) mode; partial update is not available in grayscale mode, so `display.display()` performs a full refresh.
- This is a demo web server without authentication; avoid exposing it on untrusted networks.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
