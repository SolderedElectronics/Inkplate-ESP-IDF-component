# TextBox

Demonstrates the `drawTextBox()` function for rendering multi-line text inside a defined rectangular area on Inkplate 2.

## Overview

Shows two TextBox configurations:

- **Default TextBox** — basic parameters, automatic word wrap.
- **Custom TextBox** — custom font (Roboto), text scaling, line spacing, and optional border.

If a word does not fit at the end of a row it wraps to the next line. If text exceeds the box boundary it ends with `...`.

## Hardware Required

- Soldered Inkplate 2
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Two text boxes rendered on screen: one with default styling, one with a custom Roboto font and border.

## Notes

- Always call `display.display()` after drawing to push changes to the physical panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
