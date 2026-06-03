# Deep Sleep — Partial Update

Use partial display refresh across deep sleep cycles on Inkplate 6.

## Overview

Combines deep sleep with partial e-paper updates. On each wakeup, only the changed portion of the display is refreshed rather than performing a full redraw, reducing wakeup time and display wear.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Counter or timestamp updates on screen using partial refresh each wakeup cycle. Only the changed region flickers.

## Notes

- Partial update is only available in 1-bit (BW) mode.
- Perform a full refresh periodically to prevent ghosting from accumulated partial updates.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
