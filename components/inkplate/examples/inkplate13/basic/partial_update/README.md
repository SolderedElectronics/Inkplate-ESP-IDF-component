# Partial Update

Demonstrate partial display updates on Soldered Inkplate 13SPECTRA.

## Overview

Draws an initial 8×6 grid of colored squares using a full refresh, then continuously picks a random square and updates only that region using `displayPartial()`, leaving the rest of the screen untouched.

`displayPartial(x, y, w, h, leaveOn)` accepts coordinates in the standard user space (1600×1200 px).

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

8×6 grid of colored squares on screen, with one randomly selected square updating every 3 seconds using partial refresh.

## Notes

- `leaveOn=true` keeps panel power rails on between updates for faster successive partial refreshes.
- Partial update works with all six supported colors on Inkplate 13SPECTRA.
- Perform a full refresh periodically to prevent display degradation.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
