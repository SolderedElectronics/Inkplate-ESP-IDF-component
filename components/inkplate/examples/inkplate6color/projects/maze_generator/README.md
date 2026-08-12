# Maze Generator

Generates and draws a random maze in black lines on Soldered Inkplate 6Color.

## Overview

Creates a new random maze on every boot and renders it to the e-paper display. A simple maze generation algorithm carves passages in a grid of cells, and the resulting connectivity is drawn as line segments between adjacent open cells. The maze is intended to be a printable/puzzle-style layout - you can solve it directly on the screen with an erasable marker or soft pencil.

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

A maze drawn in black lines on a white background, with an entry and exit opening. Press reset (or power-cycle) the board to generate a new random maze.

## Notes

- The maze is drawn using only black lines on a white background; the panel's color capability (green, blue, red, yellow, orange) is not used by this example.
- This example performs a single full refresh and then stays idle. Inkplate 6Color does not support partial updates.
- `cellSize` controls maze density; a smaller `cellSize` increases detail but may increase generation and drawing time.
- If writing on the panel, use only non-permanent tools (whiteboard marker, soft graphite). Avoid permanent markers to prevent staining.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
