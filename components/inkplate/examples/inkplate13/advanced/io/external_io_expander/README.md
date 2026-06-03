# External IO Expander

Control GPIO pins on the external IO expander on Soldered Inkplate 13SPECTRA.

## Overview

Demonstrates controlling an external GPIO through the on-board PCAL IO expander. Pin P1-7 (GPB7 / `IO_NUM_B7`) is configured as an output and toggled every second. Connect an LED with a 330 Ω resistor to that pin to observe the blinking.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- LED + 330 Ω resistor

## Setup

1. Connect a 330 Ω resistor to **P1-7** on the **IO Expander 2** header.
2. Connect the other end of the resistor to the LED anode (+).
3. Connect the LED cathode (−) to GND.
4. Run `idf.py menuconfig` and navigate to:
   **Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

LED on P1-7 toggles with 1-second intervals indefinitely.

## Notes

- Pin mapping: GPA0=0 … GPA7=7, GPB0=8 … GPB7=15 (`IO_NUM_B7 = 15`).
- `expander1` is the external PCAL IO expander instance defined in the board driver.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
