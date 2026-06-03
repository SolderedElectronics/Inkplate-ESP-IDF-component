# Textbox

Multi-line text wrapping in a defined box on Soldered Inkplate 6Color.

## Overview

Demonstrates how to render long text with automatic line wrapping and truncation inside a defined bounding box. Two text boxes are shown side by side: one using the default font with word wrap, and one using Roboto Light 36 pt with 27 px line spacing.

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

- Left box: sample text in the default font with word wrap.
- Right box: same text in Roboto Light 36 pt with 27 px line spacing.
- Text that exceeds the box bounds is truncated with "...".

## Notes

- Text is wrapped at word boundaries to fit within the specified rectangle.
- Some custom fonts are drawn bottom-to-top and require a vertical offset.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
