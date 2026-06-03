# Buzzer

Control the built-in buzzer on Inkplate 4TEMPERA.

## Overview

Shows basic usage of the Inkplate 4TEMPERA buzzer API with several demo sequences:

- Fixed-duration beeps using `beep(duration_ms)`
- Manual on/off control using `beepOn()` and `beepOff()`
- Frequency-controlled beeps using `beep(duration_ms, freq_hz)`

After the demo sequences, the main loop plays a repeating short melody based on the C Maj7 chord (C, E, G, B).

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

- Three short startup beeps.
- Two manual on/off beeps.
- Two low-pitch then two high-pitch beeps.
- Repeating C Maj7 melody (C–E–G–B).

## Notes

- Frequency range: ~572–2933 Hz, controlled via MCP4018 digipot. Pitch control is approximate and non-linear.
- `beep()` is a blocking call.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
