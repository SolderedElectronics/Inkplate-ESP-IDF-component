# Deep Sleep with Partial Update

Partial e-paper update combined with ESP32 deep sleep on Inkplate 4TEMPERA.

## Overview

Demonstrates the correct way to use partial screen updates together with deep sleep. Since partial updates rely on previously stored screen content in RAM (which is lost during deep sleep), the screen must be fully recreated after each wake-up before calling `partialUpdate()`.

The example uses RTC memory to preserve variables across sleep cycles, rebuilds the screen buffer on wake, and performs a partial update.

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

- First boot: full display refresh.
- Subsequent wake-ups: partial updates only.
- Counter and decimal value increment after each deep sleep cycle (every 10 seconds).

## Notes

- Partial update works only in 1-bit (black & white) mode.
- Do NOT use standard partial update examples with deep sleep without rebuilding the buffer first.
- Always rebuild screen content after deep sleep before calling `partialUpdate()`.
- Perform a full refresh every 5–10 partial updates to maintain display quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
