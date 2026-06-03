# Burn-In Clean

Run a burn-in cleaning cycle to reduce ghosting on the Inkplate 4TEMPERA e-paper panel.

## Overview

Runs the built-in burn-in cleaning sequence to remove ghosting and image retention from the e-paper panel. The screen cycles through `CLEAR_CYCLES` full black/white refresh cycles with a configurable delay between each. When cleaning is complete, "Clearing done." is shown on the display.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable

## Setup

In `main.cpp`, configure:
- `CLEAR_CYCLES` — number of cleaning cycles (default: 20)
- `CYCLES_DELAY` — delay between cycles in milliseconds (keep ≥ 5000 ms)

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The screen flashes black and white for the configured number of cycles, then shows "Clearing done."

## Notes

- Do not set `CYCLES_DELAY` below 5000 ms between cycles.
- Do not interrupt power during the cleaning sequence.
- Increase `CLEAR_CYCLES` for more severe ghosting.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
