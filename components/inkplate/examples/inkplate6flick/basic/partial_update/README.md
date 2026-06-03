# Partial Update

Demonstrate partial display updates in 1-bit mode on Soldered Inkplate 6 Flick.

## Overview

Shows how to use `partialUpdate()` to refresh only a portion of the Inkplate 6 Flick e-paper display. A counter increments and only the changed region is refreshed, resulting in faster updates with reduced flicker compared to a full refresh.

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

A counter updates on screen using partial refresh — only the changed area flickers, not the full display.

## Notes

- Partial update is only available in 1-bit (BW) mode.
- Frequent partial updates without occasional full refreshes can cause ghosting over time.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
