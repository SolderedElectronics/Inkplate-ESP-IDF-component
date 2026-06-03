# Deep Sleep — Wake Up on Touchscreen

Wake from deep sleep via WakeUp button or timer, with touchscreen initialized before sleep on Inkplate 4TEMPERA.

## Overview

Demonstrates low-power wake-up with the touchscreen controller initialized before entering sleep. Both the WakeUp button (GPIO36) and a 30-second fallback timer are configured as wake sources. A boot counter in RTC memory increments on each wake-up; the display shows the boot count and wake-up cause.

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

## How to Use

1. Flash and power on.
2. The screen shows the current boot count and wake-up reason.
3. Press the **WakeUp button** to wake, or wait 30 seconds for the timer.
4. Observe the boot count increment and the reported wake-up cause.

## Expected Output

Display shows boot count and wake-up cause: button / timer / other.

## Notes

- `bootCount` is stored with `RTC_DATA_ATTR` so it persists across deep sleep.
- Wake sources: EXT0 on GPIO36 (WakeUp button) and a 30-second timer.
- Frontlight is disabled before sleep to save power.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
