# WiFi Web Server

Use Inkplate 2 as a standalone WiFi access point and HTTP web server to send text to the display from a browser.

## Overview

Inkplate 2 creates its own WiFi access point and runs an HTTP web server. Any WiFi-capable device (PC, smartphone) can connect to the access point and open a web page to type and send text directly to the Inkplate e-paper display.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- WiFi-capable device with a web browser (PC, laptop, smartphone)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

1. Flash the firmware and power on Inkplate 2.
2. Connect your device to the Inkplate WiFi access point.
3. Open the IP address shown on the Inkplate display in a web browser.
4. Enter text into the web page and press **Send to display**.
5. The submitted text appears on the Inkplate display.

## Expected Output

- Inkplate display shows its IP address and the last received text.
- The web page allows sending custom text to the display.

## Notes

- This is a basic demonstration intended for prototyping.
- More advanced web interfaces and logic can be built on top of this example.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
