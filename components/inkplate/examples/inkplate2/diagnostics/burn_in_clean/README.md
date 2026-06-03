# Burn-In Clean

Run a burn-in cleaning cycle to reduce ghosting on the Inkplate 2 e-paper panel.

## Overview

Calls Inkplate's `burnInClean()` routine to perform a repeated full-refresh cleaning sequence designed to reduce heavy ghosting (burn-in) on the e-paper panel. When the sequence finishes, a confirmation message is shown on screen.

The number of cycles and delay between them are configurable. The process may take several minutes depending on settings.

## Hardware Required

- Soldered Inkplate 2
- USB cable

## Setup

In `main.cpp`, configure:
- `CLEAR_CYCLES` — number of cleaning refresh cycles
- `CYCLES_DELAY` — delay between cycles in milliseconds (keep ≥ 5000 ms)

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate2**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The panel repeatedly refreshes during the cleaning routine. After completion, "Clearing done." is displayed on screen.

## Notes

- Uses 1-bit (BW) display mode.
- Do not interrupt power during the cleaning sequence.
- Keep `CYCLES_DELAY` ≥ 5000 ms to avoid overstressing the panel and to allow refresh waveforms to complete.
- Effectiveness depends on panel condition and severity of ghosting; multiple runs may be required.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
