# Read Battery Voltage

Read and display the battery voltage on Soldered Inkplate 6 Flick.

## Overview

Reads the LiPo battery voltage using the Inkplate built-in ADC measurement and displays the result on the e-paper screen. Useful for monitoring battery state in portable or low-power applications.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- LiPo battery (optional — readings will show 0 V or supply voltage without a battery)

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Battery voltage in volts displayed on the Inkplate 6 Flick e-paper screen.

## Notes

- Typical LiPo range: ~3.2 V (empty) to ~4.2 V (full).
- Reading updates once at startup; loop the measurement for continuous monitoring.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
