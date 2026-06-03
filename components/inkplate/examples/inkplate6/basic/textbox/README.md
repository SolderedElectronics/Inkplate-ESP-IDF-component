# Textbox

Render wrapped text inside a bounded box on Soldered Inkplate 6.

## Overview

Demonstrates the Inkplate textbox functionality, which automatically wraps text within a defined rectangular area. Useful for displaying paragraphs, labels, or dynamic content without manual line-break management.

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

Text automatically wrapped and displayed within a defined bounding box on the Inkplate 6 e-paper display.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
