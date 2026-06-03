# Read Battery Voltage

Read and display Li-ion/Li-Po battery voltage on Soldered Inkplate 5.

## Overview

Demonstrates how to read the connected battery voltage using Inkplate's built-in battery measurement circuitry. The measured voltage is shown on the e-paper display alongside a battery icon and updated every 10 seconds.

## Hardware Required

- Soldered Inkplate 5
- USB cable
- 3.6–4.2 V Li-ion/Li-Po battery

## Setup

Connect a Li-ion/Li-Po battery to the Inkplate battery connector.

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate5**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Battery icon with measured voltage (in volts) displayed on screen, updated every 10 seconds.

## Notes

- `readBattery()` returns a `double` value in volts.
- Accuracy depends on battery condition and load.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
