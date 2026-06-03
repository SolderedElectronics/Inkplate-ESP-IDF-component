# Deep Sleep — RTC Alarm Wakeup

Wake Inkplate 6 Flick from deep sleep using a PCF85063A RTC alarm.

## Overview

Sets an alarm on the on-board PCF85063A RTC and uses it to wake Inkplate 6 Flick from deep sleep at a precise time. On each wakeup, the current RTC time is read and displayed, then a new alarm is set before returning to deep sleep.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Current time displayed on screen after each RTC alarm wakeup. Board returns to deep sleep until the next alarm fires.

## Notes

- The PCF85063A RTC continues to run during deep sleep.
- RTC time must be set before the alarm will fire correctly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
