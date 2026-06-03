# Partial Update

Partial screen update demo for Soldered Inkplate 10.

## Overview

Demonstrates partial display updates, which refresh only a defined region of the e-paper panel instead of the full screen. Partial update is significantly faster than a full refresh and reduces flicker.

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

A region of the display updates rapidly without a full-screen refresh.

## Notes

- Partial update works only in 1-bit (black & white) mode.
- Perform a full refresh every 5–10 partial updates to maintain image quality.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
