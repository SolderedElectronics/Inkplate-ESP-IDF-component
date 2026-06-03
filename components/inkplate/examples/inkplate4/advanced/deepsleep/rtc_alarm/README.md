# Deep Sleep with RTC Alarm Wake-Up

RTC alarm wake-up with deep sleep on Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the onboard PCF85063A RTC alarm interrupt to wake the board from deep sleep. On each wake cycle the display is refreshed with the current date and time, then the board returns to deep sleep until the next RTC alarm fires (every 10 seconds).

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

## Expected Output

- On first boot: RTC time and date are initialized if not already set.
- Display shows the current weekday, date, and time.
- Display refreshes automatically on every RTC alarm wake-up (every 10 seconds).

## Notes

- RTC alarm interrupt is connected to GPIO39 on Inkplate 4TEMPERA.
- Frontlight is disabled before sleep to save power.
- Deep sleep resets the ESP32; RTC values persist because the RTC runs on its own power domain.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
