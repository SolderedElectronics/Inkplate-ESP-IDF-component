# RTC — Simple

Set and display time using the PCF85063A RTC on Soldered Inkplate 6 Flick.

## Overview

Sets the on-board PCF85063A real-time clock to a defined date and time, then reads it back every second and displays the current time on the Inkplate 6 Flick e-paper screen.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

In `main.cpp`, set the initial date and time values in the `setTime()` call.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Current date and time updated and displayed on the Inkplate 6 Flick e-paper screen every second.

## Notes

- The PCF85063A RTC is battery-backed and keeps time through power cycles if a coin cell is installed.
- Time must be re-set if the backup battery is removed or depleted.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
