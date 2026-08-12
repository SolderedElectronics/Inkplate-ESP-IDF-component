# Pedometer

Step-counting pedometer with a walking animation for Soldered Inkplate 4TEMPERA.

## Overview

Uses the onboard LSM6DS3 accelerometer's embedded pedometer algorithm to
count steps as you walk with the device. The LSM6DS3 is configured for
+-2g / 26Hz operation and its embedded pedometer function is enabled by
writing directly to the sensor's control registers (`display.lsm.writeRegister()`
/ `display.lsm.readRegister()`), the same low-level register access used by
the original Arduino sketch.

The example then polls the LSM6DS3's internal 16-bit step counter register.
Whenever the count changes, the step total is redrawn on screen and a small
walking animation (`main/animationFrames.h`) advances by one frame. Most
updates use a fast partial refresh; a full refresh is performed after every
complete animation cycle to limit ghosting.

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

- E-paper: "Start walking!" is shown briefly after boot.
- "Steps taken: <number>" plus a small walking icon that changes frames as
  you walk with the device.
- The step count and animation only redraw when the LSM6DS3 reports a new
  step, so the screen stays static while the device is at rest.

## Notes

- This example uses 1-bit (black & white) display mode; partial updates are
  only supported in that mode.
- The LSM6DS3's embedded pedometer is not instantaneous — it may take a few
  steps of walking before it starts/resumes counting. This is expected
  behavior of the sensor's internal algorithm/filtering, not a bug in this
  example.
- Enabling the LSM6DS3 embedded functions (done once at startup) clears the
  step counter, so this is intentionally only performed once during
  initialization and never repeated in the polling loop.
- If the LSM6DS3 fails to configure at startup, an error is shown on screen
  and the board goes into deep sleep.
- Ported from the Soldered Inkplate Arduino library's
  `Inkplate4TEMPERA_Pedometer` example. The Arduino `setup()`/`loop()`
  structure became `app_main()` with a `while (true)` polling loop, and
  `delay()`/register access carried over almost unchanged since this
  component exposes the same `writeRegister()`/`readRegister()` primitives
  and LSM6DS3 register constants as the Arduino library.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
