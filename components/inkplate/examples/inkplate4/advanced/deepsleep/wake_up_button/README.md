# Deep Sleep — Wake Up Button

Wake from deep sleep via button press or timer on Inkplate 4TEMPERA.

## Overview

Demonstrates wake-up from ESP32 deep sleep using an external interrupt (WakeUp button on GPIO36) and a fallback 30-second timer. A boot counter stored in RTC memory increments on every wake-up and is shown on the display along with the wake-up reason.

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

1. Flash and power on — the display shows boot info and the board enters deep sleep.
2. Press the **WakeUp button** to wake immediately, or wait 30 seconds for the timer wake.
3. On each wake, the display updates with the new boot count and wake-up reason.

## Expected Output

- Inkplate display shows an incrementing boot count.
- Wake-up reason shown as either "WakeUp button" or "Timer".

## Notes

- `bootCount` is stored with `RTC_DATA_ATTR` so it persists across deep sleep.
- WakeUp button wake uses EXT0 wake-up on GPIO36.
- Timer wake interval: 30 seconds (`TIME_TO_SLEEP_US`).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
