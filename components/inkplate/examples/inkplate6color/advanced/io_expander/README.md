# IO Expander

Control IO expander GPIO pins on Soldered Inkplate 6Color.

## Overview

Demonstrates controlling a GPIO through the on-board PCAL IO expander. Pin GPB7 is configured as an output and toggled five times with 500 ms intervals, then pauses for 2 seconds before repeating. An LED connected to that pin blinks accordingly.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- LED + 330 Ω resistor

## Setup

Connect an LED + 330 Ω resistor to IO expander pin **GPB7**.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

LED on GPB7 blinks five times with 500 ms on/off intervals, then pauses 2 seconds. Repeats indefinitely.

## Notes

- `expander1` is the internal IO expander declared as an extern in `Inkplate.h`.
- Inkplate 6Color has one IO expander (internal only).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
