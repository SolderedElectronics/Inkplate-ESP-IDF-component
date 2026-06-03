# RTC — Alarm

RTC time display with alarm for Soldered Inkplate 5.

## Overview

Demonstrates how to use the PCF85063 RTC on Inkplate 5 to set time and date, configure an alarm, and display the current time on screen using partial updates. The alarm event can be detected and handled in the sketch.

## Hardware Required

- Soldered Inkplate 5
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

Set the alarm time and initial RTC time/date in `main.cpp`.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Inkplate display shows current date and time, updated via partial refresh every second.
- Alarm event is detected and handled at the configured time.

## Notes

- Inkplate 5 uses the PCF85063 RTC chip.
- Partial update works only in 1-bit (black & white) mode.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
