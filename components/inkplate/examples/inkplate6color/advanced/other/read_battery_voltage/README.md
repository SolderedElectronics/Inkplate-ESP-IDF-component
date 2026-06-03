# Read Battery Voltage

Read Li-ion/Li-Po battery voltage on Soldered Inkplate 6Color.

## Overview

Demonstrates how to read the connected battery voltage using Inkplate's built-in battery measurement circuitry and display or log the result.

## Hardware Required

- Soldered Inkplate 6Color
- USB cable
- 3.6–4.2 V Li-ion/Li-Po battery (JST connector)

## Setup

Connect a supported Li-ion/Li-Po battery to the Inkplate battery connector.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6Color**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Battery voltage value displayed on screen or printed to the serial monitor.

## Notes

- Battery voltage range: 3.6–4.2 V for a standard single-cell Li-ion/Li-Po.
- Reading may return 0 V if no battery is connected.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
