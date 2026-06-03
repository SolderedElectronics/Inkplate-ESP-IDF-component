# RTC — Simple Clock

Basic RTC time and date display for Soldered Inkplate 4TEMPERA.

## Overview

Demonstrates basic usage of the PCF85063A real-time clock (RTC) integrated on Inkplate 4TEMPERA. Shows how to set time and date, read RTC values, and display the time on the e-paper screen using partial updates.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

Set the initial RTC time and date in `main.cpp` if not already configured.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Inkplate display shows the current date and time, updated periodically using partial refresh.

## Notes

- Inkplate 4TEMPERA uses the PCF85063A RTC chip.
- Partial update works only in 1-bit (black & white) mode.
- Do not use partial update on the very first refresh after power-up.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
