# Partial Update with Deep Sleep

Incremental partial screen update across deep sleep cycles on Soldered Inkplate 10.

## Overview

Demonstrates how to combine partial display updates with deep sleep. On first boot a full refresh is performed; on subsequent wakes the ESP32 rebuilds the screen content and performs a partial update, incrementing a counter and a decimal value each cycle.

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

- First boot: full display refresh.
- Each subsequent wake-up: partial update with incremented counter and value.

## Notes

- Partial update works only in 1-bit (black & white) mode.
- Always rebuild screen content after deep sleep before calling `partialUpdate()`.
- Perform a full refresh every 5–10 partial updates to maintain image quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
