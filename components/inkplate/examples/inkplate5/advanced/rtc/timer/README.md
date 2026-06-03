# RTC — Countdown Timer

RTC countdown timer example for Soldered Inkplate 5.

## Overview

Demonstrates how to use the PCF85063A RTC countdown timer on Inkplate 5. The example sets time and date, configures the RTC timer, reads current time, and displays it via partial updates. Timer events are handled in the sketch.

## Hardware Required

- Soldered Inkplate 5
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

Configure the RTC timer interval in `main.cpp`.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows current date and time.
- Timer event is handled at the configured interval.

## Notes

- Inkplate 5 uses the PCF85063A RTC chip.
- Partial update works only in 1-bit (black & white) mode.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
