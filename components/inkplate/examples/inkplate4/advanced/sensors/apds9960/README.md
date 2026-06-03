# APDS9960 Sensor

Gesture, proximity, RGB color, and ambient light sensing with the built-in APDS9960 on Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the built-in APDS9960 sensor on Inkplate 4TEMPERA. The example enables and configures:

- **Proximity sensing** (reduced gain)
- **Gesture sensing** (Up/Down/Left/Right detection)
- **RGB color sensing**
- **Ambient light sensing**

The sketch continuously polls the sensor and updates the e-paper framebuffer only when a value changes. Partial updates provide fast, low-flicker refreshes; a full refresh is performed periodically to reduce ghosting.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

- Present a hand or object near the sensor to change proximity and trigger swipe gestures.
- Shine colored or bright light toward the sensor to change light readings.

## Expected Output

Live text fields for Proximity, Gesture, R/G/B channels, and Ambient light.

## Notes

- Partial update is supported only in 1-bit (black & white) mode.
- All APDS9960 sub-features must be explicitly enabled before use.
- This example uses polling, not interrupts.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
