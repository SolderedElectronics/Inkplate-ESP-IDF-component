# GPIO Blink

Blink an external LED using ESP32 GPIO on Inkplate 2.

## Overview

Demonstrates how to use general-purpose I/O (GPIO) pins available on the Inkplate 2 header. An external LED connected to GPIO14 is toggled every second. The e-paper display shows wiring instructions during setup.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- LED
- ~330 Ω resistor
- Jumper wires and breadboard

## Wiring

| Component | Pin |
|-----------|-----|
| LED anode | GPIO14 (via 330 Ω resistor) |
| LED cathode | GND |

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display shows "Blink example" and wiring instructions.
- LED connected to GPIO14 blinks once per second (1 s ON, 1 s OFF).

## Notes

- Use only GPIO pins exposed on the Inkplate 2 header that are not reserved for internal hardware.
- Always use a current-limiting resistor to avoid damaging the GPIO pin.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
