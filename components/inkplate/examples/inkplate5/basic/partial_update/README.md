# Partial Update

Scrolling text demo using partial display updates on Soldered Inkplate 5.

## Overview

Demonstrates partial display updates by scrolling a text string from right to left. Only the changed region is refreshed each frame, which is significantly faster than a full display update. A full refresh is forced periodically to prevent ghosting.

## Hardware Required

- Soldered Inkplate 5
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"This is partial update on Inkplate 5 e-paper display!" scrolling continuously from right to left.

## Notes

- `partialUpdate(false, true)` keeps e-paper power on for faster successive updates.
- A full refresh is triggered automatically after 9 partial updates to maintain display quality.
- Partial update is only supported in `BLACK_AND_WHITE` display mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
