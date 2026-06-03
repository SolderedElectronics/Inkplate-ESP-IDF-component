# Full Screen Colors

Display all seven colors as vertical bars on Soldered Inkplate 6Color.

## Overview

Fills the entire Inkplate 6Color screen with seven vertical color bars — one for each supported color. Useful as a quick visual test to verify that all seven e-paper colors are rendering correctly.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Seven full-height vertical bars displayed in: black, white, green, blue, red, yellow, orange.

## Notes

- Inkplate 6Color supports 7 colors: `INKPLATE_BLACK`, `INKPLATE_WHITE`, `INKPLATE_GREEN`, `INKPLATE_BLUE`, `INKPLATE_RED`, `INKPLATE_YELLOW`, `INKPLATE_ORANGE`.
- Color e-paper (ACeP) requires a full refresh; partial update is not supported.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
