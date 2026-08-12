# Mandelbrot Set

Render a Mandelbrot set view on Soldered Inkplate 6COLOR in black/white by computing each pixel on the ESP32.

## Overview

Computes and renders a Mandelbrot set region by iterating the complex function z = z^2 + c for each pixel on the display. For every (x, y) coordinate mapped into the complex plane, the code runs up to `MAXITERATIONS` iterations and decides whether the point escapes. Points that do not escape are drawn black (inside the set) and others are drawn white. Rendering happens fully in the ESP32 frame buffer and is pushed to the e-paper panel with a single full refresh.

Even though Inkplate 6COLOR can display 7 colors, this example (matching the original Arduino sketch) only ever draws black or white: its `colorAt()` helper returns a boolean result rather than mapping the iteration count to one of the 7 `INKPLATE_*` colors.

## Hardware Required

- Soldered Inkplate 6COLOR
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

A black/white Mandelbrot set rendering of the selected coordinate window. The render is CPU-intensive and can take several minutes; the image appears once complete and the board then stays idle.

## Notes

- This example draws using black/white pixel values only, even though Inkplate 6COLOR supports 7 colors (black, white, green, blue, red, yellow, orange).
- Rendering is CPU-intensive (nested loops over the full resolution with iterative complex math using double-precision floating point). Expect long runtimes and higher power draw while computing.
- `MAXITERATIONS` controls detail vs. speed. Increasing it improves boundary detail but increases render time.
- Partial updates are not used here; the whole image is generated before the first refresh. Inkplate 6COLOR does not support partial updates in any case.
- The Inkplate 6COLOR panel is 600x448 (~1.339:1), unlike the original sketch's near-square default window. Starting from the original sketch's own window, `yFrom`/`yTo` were kept unchanged and `xFrom`/`xTo` were widened around the same center point (-0.7943) so the viewing window's aspect ratio matches the panel's, keeping the fractal undistorted instead of stretched:
  - center x = (-0.7423 + -0.8463) / 2 = -0.7943
  - ySpan = 0.2102 - 0.1092 = 0.1010 (unchanged)
  - xSpan = ySpan * (600 / 448) = 0.135268
  - xFrom = center x + xSpan / 2 = -0.726666
  - xTo = center x - xSpan / 2 = -0.861934
- To explore other regions of the set, change `xFrom`/`xTo`/`yFrom`/`yTo` in `main.cpp` and reflash; keep the ratio of `(xTo - xFrom)` to `(yTo - yFrom)` equal to `E_INK_WIDTH / E_INK_HEIGHT` to avoid distortion.
- The original Arduino sketch redrew the same static view every 5 seconds forever and printed per-row progress over Serial. Since the view never changes between redraws, this port renders it once, skips the row progress printing, and stays idle instead of repeating the multi-minute computation pointlessly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
