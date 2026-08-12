# Mandelbrot Set

Render a Mandelbrot set view on Soldered Inkplate 4TEMPERA in 1-bit (BW) mode by computing each pixel on the ESP32.

## Overview

Computes and renders a Mandelbrot set region by iterating the complex function z = z^2 + c for each pixel on the display. For every (x, y) coordinate mapped into the complex plane, the code runs up to `MAXITERATIONS` iterations and decides whether the point escapes. Points that do not escape are drawn as black (inside the set) and others remain white. Rendering happens fully in the ESP32 frame buffer and is pushed to the e-paper panel with a single full refresh.

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

A black/white Mandelbrot set rendering of the selected coordinate window. The render is CPU-intensive and can take several minutes; the image appears once complete and the board then stays idle.

## Notes

- This example uses 1-bit (black & white) display mode.
- Rendering is CPU-intensive (nested loops over the full resolution with iterative complex math using double-precision floating point). Expect long runtimes and higher power draw while computing.
- `MAXITERATIONS` controls detail vs. speed. Increasing it improves boundary detail but increases render time.
- Partial updates are not used here; the whole image is generated before the first refresh.
- To explore other regions of the set, change `xFrom`/`xTo`/`yFrom`/`yTo` in `main.cpp` and reflash.
- The original Arduino sketch redrew the same static view every 5 seconds forever. Since the view never changes between redraws, this port renders it once and stays idle instead of repeating the multi-minute computation pointlessly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
