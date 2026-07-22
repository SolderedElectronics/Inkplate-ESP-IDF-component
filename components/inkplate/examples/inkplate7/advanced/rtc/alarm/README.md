# RTC — Alarm

Trigger an action from a PCF85063A RTC alarm on Soldered Inkplate 7.

## Overview

Sets a one-shot alarm on the PCF85063A RTC. When the alarm time is reached, the alarm flag is detected by polling and a message is displayed on the Inkplate 7 e-paper screen.

## Hardware Required

- Soldered Inkplate 7
- USB cable

## Setup

In `main.cpp`, configure the alarm time in the `setAlarm()` call.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate7**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

"Alarm triggered!" (or equivalent) displayed on the Inkplate 7 e-paper screen when the RTC alarm fires.

## Notes

- The alarm flag must be cleared after detection to avoid immediate re-trigger.
- For interrupt-driven wakeup, see the `interrupt_alarm` example.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
