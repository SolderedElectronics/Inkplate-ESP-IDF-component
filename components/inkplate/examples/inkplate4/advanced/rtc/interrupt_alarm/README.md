# RTC — Interrupt Alarm

RTC alarm with hardware interrupt handling for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the PCF85063A RTC alarm functionality together with its interrupt output on Inkplate 4TEMPERA. The example sets time and date, configures an alarm, reads current time, displays it via partial updates, and handles the RTC interrupt event in software.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

Set the alarm time and initial RTC time/date in `main.cpp`.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows current date and time.
- "ALARM" text appears when the RTC alarm interrupt fires.

## Notes

- Inkplate 4TEMPERA uses the PCF85063A RTC chip.
- RTC interrupt is connected to GPIO39 on Inkplate 4TEMPERA.
- Partial update works only in 1-bit (black & white) mode.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
