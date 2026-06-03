# IO Expanders

Control internal and external IO expander GPIO pins on Soldered Inkplate 10.

## Overview

Demonstrates how to use both IO expanders available on Inkplate 10. The example alternates blinking an LED connected to the external IO expander (IO Expander 2) and an LED connected to the internal IO expander (IO Expander 1), showing correct pin addressing for each.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- 2 × LED + 330 Ω resistor

## Setup

1. Connect one LED + 330 Ω resistor to **P1-7 (GPB7)** on **IO Expander 2** (external).
2. Connect another LED + 330 Ω resistor to **P1-7 (GPB7)** on **IO Expander 1** (internal).

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- External IO expander LED blinks for 5 seconds.
- Internal IO expander LED blinks for 5 seconds.
- Sequence repeats continuously.

## Notes

- **Internal IO expander (expander1):** DO NOT use pins GPA0–GPA7 or GPB0 (IO_NUM_A0–IO_NUM_B0). These pins are reserved by the display driver. Use only pins 9–15 (GPB1–GPB7). Using restricted pins may permanently damage the display.
- **External IO expander (expander2):** All pins are free to use.
- Pin mapping: GPA0=0 … GPA7=7, GPB0=8 … GPB7=15 (`IO_NUM_B7`).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
