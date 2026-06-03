# RTC Calibration

RTC crystal calibration utility for Soldered Inkplate 10.

## Overview

Provides a step-by-step procedure for calibrating the PCF85063A RTC crystal oscillator on Inkplate 10. After pressing the WakeUp button, the RTC is zeroed and elapsed time is displayed every second. The internal capacitor and clock offset registers can be adjusted to correct drift.

## Hardware Required

- Soldered Inkplate 10
- USB cable
- Optional: oscilloscope for measuring RTC crystal frequency

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Calibration Procedure

1. Optionally comment out `setClockOffset()` and `setInternalCapacitor()` for the first run to measure raw drift.
2. Build and flash.
3. Press the **WakeUp button** (GPIO36) to start the timer.
4. After 2-3 days, compare the displayed time to actual time.
5. Calculate the correction offset as described in the source comments.
6. Set the calculated offset and rebuild.

## Expected Output

- "Press wake button to start RTC!" shown on screen.
- HH:MM:SS displayed after button press, updating every second.

## Notes

- WakeUp button is connected to GPIO36.
- See PCF85063A datasheet section 8.2.3 for offset register details.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
