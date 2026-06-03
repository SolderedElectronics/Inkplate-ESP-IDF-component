# Accelerometer & Gyroscope

Visualize motion from the built-in LSM6DS3 IMU as a rotating 3D cube on Inkplate 4TEMPERA.

## Overview

Demonstrates how to use the built-in LSM6DS3 accelerometer and gyroscope. Raw accelerometer and gyroscope axes (X/Y/Z) are read continuously. Numeric readings are printed on the lower half of the display, and a wireframe cube is rendered on the upper half — its rotation angles derived from accelerometer values and smoothed frame-to-frame.

Each cube edge is projected from 3D to 2D using basic rotation matrices with a simple perspective projection. Partial updates keep the animation responsive; a full refresh is forced periodically to reduce ghosting.

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

## How to Use

Flash and power on. Tilt and rotate the device — the cube rotation changes with acceleration.

## Expected Output

- A wireframe cube near the center of the display, rotating as the device is moved.
- Live text readouts for ACC X/Y/Z and GYRO X/Y/Z.

## Notes

- Partial update is supported only in 1-bit (black & white) mode.
- This is a visualization example, not a calibrated orientation filter.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
