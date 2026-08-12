# Calculator

Touchscreen calculator app for Soldered Inkplate 4TEMPERA.

## Overview

Draws an on-screen calculator keypad (digits 0-9, +, -, x, /, ., =) plus
Refresh / Clear Input / Clear result buttons, and lets you enter numbers and
perform the four basic arithmetic operations entirely via the onboard
touchscreen — no network connection is used.

Each button is a rectangle checked with `touchInArea()`; the keypad geometry
and the drawing helper (`mainDraw()`) live in `main/Calculator.h`, while
`main/main.cpp` holds the calculator's state machine (digit/operator entry
and evaluation) and the touch polling loop. Most interactions redraw the UI
with `partialUpdate()` for a fast, low-flicker response; the "Refresh"
button performs a full `display()` refresh instead. The last completed
`expression = result` is kept on screen as a simple history line until
cleared.

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

- E-paper: a calculator UI with a numeric/operator keypad, a current-entry
  line, and a history/result line.
- Tapping a digit or operator appends it to the current expression and
  redraws via a partial update.
- Tapping "=" evaluates the expression (when an operator is set and the
  right-hand number is non-zero) and shows `expression = result` on the
  history line.
- "Clear Input" resets the current entry; "Clear result" erases the history
  line; "Refresh" redraws the full UI with a full e-paper refresh.

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
- Ported from the Soldered Inkplate Arduino library's
  `Inkplate4TEMPERA_Calculator` example. The original's auto-generated,
  numbered `rectN_*`/`textN_*` globals were consolidated into small
  `KeypadKey` tables in `Calculator.h`; the `String` expression/history
  buffers became `std::string`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
