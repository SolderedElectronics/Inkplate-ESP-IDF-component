# Set Waveform

Compare EPD waveforms on Soldered Inkplate 10 to select the best for your application.

## Overview

Iterates through e-paper waveforms 1-5, applying each one and rendering a full grayscale gradient so you can visually evaluate contrast, banding, and ghosting for each waveform. Use this to select the waveform that best suits your application.

## Hardware Required

- Soldered Inkplate 10
- USB cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate10**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- The display cycles through waveforms 1-5 automatically, every 5 seconds.
- Each iteration shows a grayscale gradient (8 shades) and the active waveform number.

## Notes

- Display mode is 3-bit grayscale (8 shades).
- Different waveforms trade off between refresh speed, ghosting, and gray level accuracy.
- Waveform selection is not persisted; call `setWaveform()` at startup in your production code.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
