# Game of Life

Conway's Game of Life animation using partial updates for Soldered Inkplate 4TEMPERA.

## Overview

Implements Conway's Game of Life on the Inkplate 4TEMPERA e-paper display. The screen is divided into a grid of square cells (randomized cell size each run), and each generation updates the grid according to the classic Life rules. Only the cells that changed state are redrawn into the frame buffer, and the sketch uses `partialUpdate()` for most frames to keep the animation smooth on e-paper. A full refresh is performed periodically to reduce ghosting, and the grid is automatically re-randomized whenever the simulation stagnates (too little change over time).

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A continuous Game of Life animation using black/white cells. New cells appear as filled black squares; older cells are drawn with a shrinking white interior to indicate their age. The grid re-randomizes itself whenever activity drops.

## Notes

- This example uses 1-bit (black & white) display mode; partial updates are only supported in `BLACK_AND_WHITE` mode.
- A full refresh is performed every `FULLREFRESH` (40) frames to reduce ghosting from repeated partial updates.
- Computation and drawing happen in the same pass (`stepGeneration()`), matching the original sketch's design: there is no separate offscreen render step, only the cells that changed state between generations are redrawn.
- The simulation uses two in-RAM grids sized for the minimum cell size (`MIN_CELLSZ`), so RAM usage increases with display resolution and the chosen cell size range.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
