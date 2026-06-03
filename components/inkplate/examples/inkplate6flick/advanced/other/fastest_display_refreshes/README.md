# Fastest Display Refreshes

Benchmark maximum display refresh rate on Soldered Inkplate 6 Flick.

## Overview

Measures and displays the time taken for a full display refresh at maximum speed. Helps characterize display performance for latency-sensitive applications by reporting the refresh duration in milliseconds on the serial monitor.

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

Refresh time in milliseconds printed to the serial monitor for each display update cycle.

## Notes

- Refresh time varies between 1-bit and 4-bit grayscale modes.
- Do not exceed the recommended refresh rate; excessive updates can shorten panel lifespan.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
