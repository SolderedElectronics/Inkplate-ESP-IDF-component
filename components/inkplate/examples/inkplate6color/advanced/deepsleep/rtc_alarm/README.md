# RTC Alarm Deep Sleep

Wake from deep sleep on a PCF85063A RTC alarm on Soldered Inkplate 6Color.

## Overview

Sets a PCF85063A RTC alarm to wake the ESP32 from deep sleep every 10 seconds. On each wake-up the display is refreshed with the current date and time, then the board returns to sleep.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Current date and time shown on screen, updating every 10 seconds.

## Notes

- RTC alarm interrupt is connected to GPIO39 on Inkplate 6Color.
- If the RTC loses power, it re-initializes to the hardcoded time.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
