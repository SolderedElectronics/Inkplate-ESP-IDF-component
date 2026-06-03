# Deep Sleep — Simple

Put Inkplate 6 into deep sleep and wake on a timer.

## Overview

Demonstrates basic ESP32 deep sleep with timed wakeup. Inkplate 6 draws a message and a wake counter on the display, then enters deep sleep. On each wakeup the counter (stored in RTC memory) increments and the display is updated before sleeping again.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display shows wake count and sleep duration. Board sleeps and wakes repeatedly, updating the counter each cycle.

## Notes

- Wake count is stored in `RTC_DATA_ATTR` memory and survives deep sleep.
- Deep sleep current draw is significantly lower than active mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
