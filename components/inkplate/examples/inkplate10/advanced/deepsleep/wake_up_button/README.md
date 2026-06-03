# Wake-Up Button Deep Sleep

Wake from deep sleep via button press or timer on Soldered Inkplate 10.

## Overview

Demonstrates two deep sleep wake-up sources: the WakeUp button (GPIO36, EXT0) and a timer. The boot count is stored in RTC memory and increments across deep sleep cycles. Each wake-up updates the display with the current boot count and wake-up reason.

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

- Display shows an incrementing boot count.
- Wake-up reason shown as either "WakeUp button" or "timer".

## Notes

- WakeUp button uses EXT0 wake-up on GPIO36.
- `bootCount` is stored in RTC memory (`RTC_DATA_ATTR`) and survives deep sleep.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
