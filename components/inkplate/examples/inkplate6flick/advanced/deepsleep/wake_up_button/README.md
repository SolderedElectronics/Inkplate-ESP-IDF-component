# Deep Sleep — Wake Up Button

Wake Inkplate 6 Flick from deep sleep using the WakeUp button.

## Overview

Puts Inkplate 6 Flick into deep sleep and configures the WakeUp button (GPIO36) as an external wakeup source. Each button press wakes the board, updates a wake counter on the display, and returns to deep sleep.

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

Display updates with an incremented wake counter each time the WakeUp button is pressed.

## Notes

- WakeUp button is connected to GPIO36.
- Wake count persists across deep sleep cycles via `RTC_DATA_ATTR`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
