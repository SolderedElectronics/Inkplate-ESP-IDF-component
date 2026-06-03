# NTP Alarm Clock with Deep Sleep

NTP-synced alarm clock on Inkplate 2 with deep sleep between wake-up checks.

## Overview

Implements a simple alarm clock using NTP-synced time and ESP32 deep sleep. On each wake cycle the board connects to WiFi, fetches the current time via NTP, and checks whether the configured alarm time has been reached.

- If the alarm has not triggered: shows a waiting screen and enters deep sleep for `wakeMinutes`.
- If the alarm time is reached or passed: shows an alarm screen and stays awake.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- WiFi connection with Internet access

## Setup

### 1. Configure WiFi credentials

Run `idf.py menuconfig` and navigate to:
**WiFi Configuration → Enter your SSID and password**

### 2. Configure board, alarm, and timezone

In `idf.py menuconfig`:
- **Inkplate Boards → Inkplate2**

In the sketch (`main.cpp`), set:
- `TIME_ZONE` — UTC offset in hours (e.g. `1` for UTC+1)
- `alarmHour`, `alarmMins`, `alarmSecs`, `alarmDay`, `alarmMon` — alarm time and date
- `wakeMinutes` — how often (in minutes) to wake and check the alarm

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display shows "Waiting for HH:MM on DD.MM." until the alarm time is reached.
- When alarm fires: "ALARM!" is shown and the device stays awake.

## Notes

- Uses 1-bit (BW) display mode with full refresh.
- WiFi reconnects on every wake cycle, increasing wake time and power use. For lower power, consider an RTC-only intermediate check without WiFi.
- If the alarm time is already in the past on first boot, the alarm screen shows immediately.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
