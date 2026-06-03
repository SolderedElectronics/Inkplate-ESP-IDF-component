# RTC — Interrupt Alarm

Wake Inkplate 6 from deep sleep using a PCF85063A RTC interrupt alarm.

## Overview

Configures the PCF85063A RTC to generate an interrupt signal on its INT pin when an alarm fires. The interrupt is wired to a GPIO that triggers ESP32 external wakeup from deep sleep, allowing precise timed wakeup with minimal power consumption.

## Hardware Required

- Soldered Inkplate 6
- USB cable

## Setup

In `main.cpp`, configure the alarm time in the `setAlarm()` call.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Board wakes from deep sleep when the RTC alarm fires, displays a message, sets the next alarm, then returns to sleep.

## Notes

- The RTC INT pin is connected to an ESP32 GPIO configured for `ext0` wakeup.
- The alarm flag must be cleared on wakeup before setting the next alarm.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
