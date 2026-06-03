# Fuel Gauge

Read battery statistics from the built-in BQ27441 fuel gauge on Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the built-in BQ27441-G1A fuel gauge on Inkplate 4TEMPERA. Battery statistics are read and displayed every 2 seconds using partial updates; a full refresh is forced periodically to reduce ghosting.

Measurements:
- State of charge (SoC %)
- Voltage (mV)
- Average current (mA)
- Full and remaining capacity (mAh)
- Average power draw (mW)
- State of health (SoH %)

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB cable
- Li-Ion battery

## Setup

In `main.cpp`, set `BATTERY_CAPACITY` to your battery capacity in mAh.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Text lines for SoC, voltage, current, full/remaining capacity, power draw, and state of health — updated every ~2 seconds.

## Notes

- Partial update is supported only in 1-bit (black & white) mode.
- Set `BATTERY_CAPACITY` to match your battery for accurate readings.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
