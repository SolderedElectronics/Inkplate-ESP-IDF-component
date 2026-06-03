# NTP Clock

Display current time and date fetched from the Internet via NTP on Inkplate 2.

## Overview

Connects to WiFi, syncs time via NTP, then continuously displays the current time (HH:MM) and date (DD.MM.YYYY) on the e-paper panel, refreshing every `DELAY_TIME_MS` milliseconds.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- WiFi connection with Internet access

## Setup

### 1. Configure WiFi credentials

Run `idf.py menuconfig` and navigate to:
**WiFi Configuration → Enter your SSID and password**

### 2. Configure board and timezone

In `idf.py menuconfig`:
- **Inkplate Boards → Inkplate2**
- Set `TIME_ZONE` in the sketch to your UTC offset (e.g. `1` for UTC+1).
- Set `DELAY_TIME_MS` for the refresh interval.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

Display shows large HH:MM time and DD.MM.YYYY date, refreshing periodically.

## Notes

- Uses 1-bit (BW) display mode with full refresh.
- Frequent refreshes increase e-paper flashing; use longer intervals for battery-powered use.
- WiFi must remain connected for each NTP fetch.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
