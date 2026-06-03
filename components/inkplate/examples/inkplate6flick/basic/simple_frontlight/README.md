# Simple Frontlight

Control the integrated frontlight on Soldered Inkplate 6 Flick.

## Overview

Demonstrates how to enable and control the frontlight on the Inkplate 6 Flick. Brightness is adjusted by sending characters through the serial monitor (UART0 at 115200 baud): `+` increases brightness, `-` decreases it, and `s` triggers a light animation sweep.

## Hardware Required

- Soldered Inkplate 6 Flick with integrated frontlight
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Usage

Open a serial terminal at **115200 baud** and send:
- `+` — increase frontlight brightness
- `-` — decrease frontlight brightness
- `s` — run a frontlight animation sweep

## Expected Output

- Frontlight turns on at startup.
- Current brightness level (0–63) printed to serial after each change.

## Notes

- Frontlight brightness range is 0–63.
- `display.frontlight.setState(true)` enables the frontlight driver circuit.
- `display.frontlight.setBrightness(value)` sets the brightness level.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
