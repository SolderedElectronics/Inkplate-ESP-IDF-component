# RTC — Countdown Timer

RTC countdown timer example for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the PCF85063A RTC countdown timer on Inkplate 4TEMPERA. The example sets time and date, configures the RTC timer, reads current time values, and displays them via partial updates. When the timer fires, "Timer!" is shown on screen.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

Configure the RTC timer interval in `main.cpp`.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows current date and time.
- "Timer!" text appears when the countdown timer fires.

## Notes

- Inkplate 4TEMPERA uses the PCF85063A RTC chip.
- Partial update works only in 1-bit (black & white) mode.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
