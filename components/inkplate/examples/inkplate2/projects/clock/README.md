# Clock

Multi-style clock (digital, binary, or analog) for Soldered Inkplate 2, synced over WiFi/NTP.

## Overview

Connects Inkplate 2 to a WiFi network, syncs the system clock over NTP, and draws one of three clock styles on the e-paper screen:

- **Digital** — four large 7-segment style digits (HH:MM) drawn from bitmap assets, with a black colon separator.
- **Binary** — hour, minute, day, and month shown as columns of binary dots (filled red = 1, black outline = 0).
- **Analog** — a clock face with a black hour hand and a red minute hand.

The clock redraws and does a full e-paper refresh every `REFRESH_INTERVAL_MS` (default 5 minutes).

## Hardware Required

- Soldered Inkplate 2
- USB cable
- WiFi network with Internet access (for NTP)

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate2**
- **Example Configuration → WiFi SSID/Password** (enter your credentials)

Then, in `main/main.cpp`, adjust to taste:
- `CLOCK_MODE` — 0 = digital, 1 = binary, 2 = analog (default: 1)
- `TIMEZONE_OFFSET_HOURS` — your local UTC offset, e.g. `2` for UTC+2 (**TODO**, default: 2)

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- **CLOCK_MODE 0 (digital):** large HH:MM digits in red with a black colon separator.
- **CLOCK_MODE 1 (binary):** four columns showing HH, MM, DD, MM (month) as binary circles with labels and bit-value markers (8/4/2/1).
- **CLOCK_MODE 2 (analog):** a clock face with a black hour hand and a red minute hand.
- On WiFi failure: an error message is shown on the display and the example stops (reset the board to try again).

## Notes

- Inkplate 2 has no onboard RTC chip (unlike Inkplate 4/5/6/10/13), so this example relies purely on WiFi + NTP for timekeeping via `display.wifi.setCurrentTime()`. Time is not retained across a power cycle without a network connection.
- Inkplate 2 does not support partial updates, so every redraw is a full refresh via `display.display()`.
- The original Arduino example used deep sleep between updates to save power. This port keeps the ESP32 awake and simply delays in a loop (`vTaskDelay`) between redraws instead, matching this component's other periodic-update examples; deep sleep is out of scope for this port.
- `display.wifi.setCurrentTime()` sets the system `TZ` environment variable internally, but `time()` always returns UTC seconds regardless of `TZ`. This example applies `TIMEZONE_OFFSET_HOURS` manually instead (same fixed-offset approach as the original sketch), so daylight saving time is not handled automatically.
- Bitmap digits for the digital clock were pre-generated with the Soldered Image Converter and are included under `main/fonts/`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
