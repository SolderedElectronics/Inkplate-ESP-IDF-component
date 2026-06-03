# RTC Alarm

Set a PCF85063A RTC alarm and trigger an action on Soldered Inkplate 6Color.

## Overview

Demonstrates how to configure an alarm on the PCF85063A RTC. When the alarm time is reached, an interrupt fires and the Inkplate 6Color display updates to show the alarm event.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable

## Setup

In `main.cpp`, set the desired alarm time before building.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display updates when the RTC alarm fires, showing the alarm event.

## Notes

- RTC alarm interrupt is connected to GPIO39 on Inkplate 6Color.
- The alarm fires once; set a new alarm to repeat.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
