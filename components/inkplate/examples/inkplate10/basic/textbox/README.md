# Textbox

Multi-line text wrapping demo for Soldered Inkplate 10.

## Overview

Demonstrates how to render long text with automatic line wrapping inside a defined bounding box on the Inkplate 10 display.

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

A block of text with automatic line wrapping displayed within the defined area on the e-paper screen.

## Notes

- Text is wrapped at word boundaries to fit within the specified rectangle.
- Adjust text size and box dimensions to fit your content.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
