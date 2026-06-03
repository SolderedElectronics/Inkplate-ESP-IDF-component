# IO Expander

Control GPIO pins on the internal IO expander on Inkplate 5.

## Overview

Demonstrates how to use the internal IO expander (PCAL) available on Inkplate 5. An LED connected to pin P1-7 (GPB7 / `IO_NUM_B7`) on the internal expander is blinked every second.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- LED
- 330 Ω resistor

## Wiring

| Component | Pin |
|-----------|-----|
| LED anode | IO expander P1-7 (GPB7) via 330 Ω resistor |
| LED cathode | GND |

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

LED connected to `IO_NUM_B7` blinks once per second continuously.

## Notes

- **WARNING:** DO NOT use GPA0–GPA7 or GPB0 on the internal expander — using restricted pins may permanently damage the display.
- Use only pins 9–15 (P1-1 to P1-7, i.e. `IO_NUM_B1` to `IO_NUM_B7`).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
