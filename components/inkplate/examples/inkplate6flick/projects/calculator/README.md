# Calculator

Touchscreen calculator app for Soldered Inkplate 6 Flick.

## Overview

Draws an on-screen calculator keypad (digits 0-9, +, -, x, /, ., =) plus
Refresh / Clear / Clear history buttons, and lets you enter numbers and
perform the four basic arithmetic operations entirely via the onboard
touchscreen — no network connection is used.

Each button is a rectangle checked with `touchInArea()`; the keypad geometry
and the drawing helper (`mainDraw()`) live in `main/Calculator.h`, while
`main/main.cpp` holds the calculator's state machine (digit/operator entry
and evaluation) and the touch polling loop. Most interactions redraw the UI
with `partialUpdate()` for a fast, low-flicker response; the "Refresh"
button performs a full `display()` refresh instead. Completed calculations
are appended to a running, multi-line history panel until it is cleared.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- E-paper: a calculator UI with a numeric/operator keypad, a current-entry
  line, and a history panel.
- Tapping a digit or operator appends it to the current expression and
  redraws via a partial update.
- Tapping "=" evaluates the expression (when an operator is set and the
  right-hand number is non-zero) and appends `expression = result` as a new
  line in the history panel.
- "Clear" (top row) resets the current entry; "Clear history" (top-left
  corner) erases the history panel; "Refresh" redraws the full UI with a
  full e-paper refresh.

## Notes

- This example uses 1-bit (black & white) display mode; partial updates are
  only supported in that mode.
- For best image quality, perform a full refresh periodically (the
  "Refresh" button); repeated partial updates can leave artifacts on
  e-paper.
- Division by zero is guarded: the right operand must be non-zero to
  trigger the "=" action.
- Numbers are limited to 6 digits with at most 2 decimal digits, matching
  the original sketch's input limits.
- `touchInArea()` polls the touchscreen controller internally, so no
  separate "read touch" step is needed before checking each button's area.
- The touchscreen (Cypress CY8CTMA140 controller, via `TouchCypress`) is
  initialized and powered on automatically as part of constructing
  `Inkplate display;` — no explicit `touchscreen.init()` call is needed on
  this port. This differs from the original `.ino`, which called
  `display.touchscreen.init(true)` in `setup()`.
- `touchInArea(x, y, w, h)` has the same call signature as the Inkplate
  4TEMPERA (TouchElan) port of this example, but is backed by a different
  driver here (TouchCypress, for the CY8CTMA140 controller used on
  Inkplate 6 Flick) — see `include/features/TouchCypress.h`.
- Board geometry (keypad/button coordinates) uses the 1024x758 layout from
  the original Inkplate 6 Flick sketch, not the smaller layout used by the
  Inkplate 4TEMPERA port of this same example.
- Ported from the Soldered Inkplate Arduino library's
  `Inkplate6FLICK_Calculator` example. The original's auto-generated,
  numbered `rectN_*`/`textN_*` globals were consolidated into small
  `KeypadKey` tables in `Calculator.h`; the `String` expression/history
  buffers became `std::string`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
