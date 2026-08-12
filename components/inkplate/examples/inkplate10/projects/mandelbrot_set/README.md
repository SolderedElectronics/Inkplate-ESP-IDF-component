# Mandelbrot Set

Render a Mandelbrot set view on Soldered Inkplate 10 in 1-bit (BW) mode by computing each pixel on the ESP32.

## Overview

Computes and renders a Mandelbrot set region by iterating the complex function z = z^2 + c for each pixel on the display. For every (x, y) coordinate mapped into the complex plane, the code runs up to `MAXITERATIONS` iterations and decides whether the point escapes. Points that do not escape are drawn as black (inside the set) and others remain white. Rendering happens fully in the ESP32 frame buffer and is pushed to the e-paper panel with a single full refresh.

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

A black/white Mandelbrot set rendering of the selected coordinate window. The render is CPU-intensive and can take several minutes; the image appears once complete and the board then stays idle.

## Notes

- This example uses 1-bit (black & white) display mode.
- Rendering is CPU-intensive (nested loops over the full resolution with iterative complex math using double-precision floating point). Expect long runtimes and higher power draw while computing.
- `MAXITERATIONS` controls detail vs. speed. Increasing it improves boundary detail but increases render time.
- Partial updates are not used here; the whole image is generated before the first refresh.
- The Inkplate 10 panel is 1200x825 (~1.4545:1, i.e. 16:11), unlike the original sketch's near-square default window. `xFrom`/`xTo` were widened around the same center point used by the original sketch so the viewing window's aspect ratio matches the panel's, keeping the fractal undistorted instead of stretched vertically. The math:
  - center x = (-0.7423 + -0.8463) / 2 = **-0.7943**
  - original height = yTo - yFrom = 0.2102 - 0.1092 = **0.1010** (kept unchanged)
  - target aspect = E_INK_WIDTH / E_INK_HEIGHT = 1200 / 825 = 16 / 11 ≈ **1.454545**
  - new width = height × aspect = 0.1010 × 16 / 11 ≈ **0.146909**
  - half width = new width / 2 ≈ **0.073455**
  - `xFrom` = center + half width ≈ -0.7943 + 0.073455 = **-0.720845**
  - `xTo` = center - half width ≈ -0.7943 - 0.073455 = **-0.867755**
  - `yFrom` / `yTo` are unchanged from the original sketch: **0.1092** / **0.2102**
- To explore other regions of the set, change `xFrom`/`xTo`/`yFrom`/`yTo` in `main.cpp` and reflash; keep the ratio of `(xTo - xFrom)` to `(yTo - yFrom)` equal to `E_INK_WIDTH / E_INK_HEIGHT` to avoid distortion.
- The original Arduino sketch redrew the same static view every 5 seconds forever. Since the view never changes between redraws, this port renders it once and stays idle instead of repeating the multi-minute computation pointlessly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
