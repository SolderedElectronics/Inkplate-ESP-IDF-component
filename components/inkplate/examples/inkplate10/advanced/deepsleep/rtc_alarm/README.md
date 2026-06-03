# RTC Alarm Deep Sleep

Wake from deep sleep on a PCF85063A RTC alarm on Soldered Inkplate 10.

## Overview

Sets a PCF85063A RTC alarm to wake the ESP32 from deep sleep at a configured interval. On each wake-up the display is refreshed with the current weekday, date, and time, then the board returns to sleep.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Inkplate display shows the current weekday, date, and time, refreshing automatically on each RTC alarm wake-up.

## Notes

- RTC alarm interrupt is connected to GPIO39 on Inkplate 10.
- All application logic must be in `app_main()`; deep sleep is entered at the end.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
