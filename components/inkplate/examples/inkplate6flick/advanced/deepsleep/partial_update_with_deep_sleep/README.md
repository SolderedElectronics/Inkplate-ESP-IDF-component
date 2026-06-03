# Deep Sleep — Partial Update with Deep Sleep

Use partial display refresh correctly across deep sleep cycles on Inkplate 6 Flick.

## Overview

Demonstrates how to correctly combine partial screen updates with ESP32 deep sleep. Since partial updates rely on previously stored screen content in RAM (which is erased during deep sleep), the screen must be rebuilt after waking up before calling `partialUpdate()`. This example shows how to:

1. Store state in RTC memory to survive deep sleep.
2. Rebuild the framebuffer from saved state after wakeup (`preloadScreen()`).
3. Safely perform partial updates.

The board wakes every 10 seconds, increments a counter and multiplies a decimal value, then updates only the changed region before sleeping again.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- First boot performs a full refresh.
- Subsequent wakeups perform partial updates only.
- Counter increments and decimal multiplies by 1.23 after each sleep cycle.

## Notes

- Partial update is only available in 1-bit (BW) mode.
- Always call `preloadScreen()` after deep sleep before calling `partialUpdate()` — skipping it will corrupt the display.
- Perform a full refresh every 5–10 partial updates to maintain image quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
