# Full Screen Colors

Display a full-screen color bar test on Soldered Inkplate 13SPECTRA.

## Overview

Fills the entire Inkplate 13SPECTRA screen with six vertical color bars — one for each supported color. Useful as a quick visual test to verify all six e-paper colors render correctly.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Six full-height vertical bars in: black, white, yellow, red, blue, green — across the 1600×1200 px display.

## Notes

- Inkplate 13SPECTRA supports 6 colors: `INKPLATE_BLACK`, `INKPLATE_WHITE`, `INKPLATE_YELLOW`, `INKPLATE_RED`, `INKPLATE_BLUE`, `INKPLATE_GREEN`.
- A full refresh is required to display all colors correctly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
