# RTC — Timer

Use the PCF85063A countdown timer on Soldered Inkplate 6.

## Overview

Configures the PCF85063A RTC countdown timer to fire at a regular interval. When the timer expires, its flag is detected and a counter is incremented and displayed on the Inkplate 6 e-paper screen. The timer is then reloaded for the next interval.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

In `main.cpp`, set the timer interval using the `setTimer()` call.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Counter increments on screen each time the RTC countdown timer expires.

## Notes

- The PCF85063A timer supports intervals from fractions of a second to hours.
- Timer flag must be cleared after each expiry to allow the next countdown.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
