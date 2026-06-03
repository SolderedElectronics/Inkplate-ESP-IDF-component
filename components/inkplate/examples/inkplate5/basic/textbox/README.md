# TextBox

Text box with word wrap example for Soldered Inkplate 5.

## Overview

Demonstrates the `drawTextBox()` function which renders long text within a defined rectangular region with automatic word wrapping. Two text boxes are shown side by side:

- **Left box** — default built-in font with word wrap.
- **Right box** — custom Roboto Light 36 pt font with configurable line spacing.

Text that exceeds the box bounds is truncated with `...`.

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

Two text boxes with sample text side by side: one in the default font, one in Roboto Light 36 pt with 27 px line spacing.

## Notes

- Text that exceeds the box bounds is truncated with `...`.
- Some custom fonts are drawn bottom-to-top and require a vertical offset.
- Always call `display.display()` after drawing to push changes to the physical panel.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
