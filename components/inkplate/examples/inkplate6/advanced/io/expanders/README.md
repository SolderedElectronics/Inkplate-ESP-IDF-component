# IO Expanders

Control GPIO pins on both internal and external IO expanders on Soldered Inkplate 6.

## Overview

Demonstrates how to use GPIO pins on both the internal IO expander (IO Expander 1) and the external IO expander (IO Expander 2) on Inkplate 6. The example alternates blinking an LED connected to the external expander for 5 seconds, then an LED on the internal expander for 5 seconds, repeating continuously.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- 2× LED + 330 Ω resistor

## Setup

1. Connect one LED + 330 Ω resistor to **P1-7 (GPB7)** on **IO Expander 2** (external).
2. Connect another LED + 330 Ω resistor to **P1-7 (GPB7)** on **IO Expander 1** (internal).
3. Run `idf.py menuconfig` and navigate to:
   **Inkplate Boards → Inkplate6**

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

- **External IO expander (IO Expander 2):** all pins are free to use.
- **Internal IO expander (IO Expander 1):** do **not** use GPA0–GPA7 or GPB0 — these are reserved for display control. Using restricted pins may permanently damage the display. Use only pins GPB1–GPB7 (P1-1 to P1-7).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
