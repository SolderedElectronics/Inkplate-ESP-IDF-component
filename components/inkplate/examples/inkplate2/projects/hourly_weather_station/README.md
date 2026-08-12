# Hourly Weather Station

Battery-friendly hourly weather dashboard for Soldered Inkplate 2, powered by the OpenWeatherMap API.

## Overview

Connects Inkplate 2 to WiFi, synchronizes time via NTP, and fetches an hourly forecast from the OpenWeatherMap "One Call" API for a configured latitude/longitude. The dashboard renders three framed panels: the current temperature/condition icon and the next two hours. After each refresh the board goes into deep sleep and repeats the cycle - because deep sleep resets the ESP32, execution always restarts from `app_main()`.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- Stable WiFi connection
- An OpenWeatherMap API key (free tier, "One Call API 3.0")

## Setup

### 1. Get an OpenWeatherMap API key

1. Create a free account at https://openweathermap.org/guide and subscribe to the "One Call API 3.0" plan.
2. Copy your API key into `API_KEY` in `main/main.cpp`.
3. Set `LOCATION_LAT` / `LOCATION_LON` in `main/main.cpp` to your location's coordinates.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate2**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: three framed panels showing "Now" and the next two hour labels, a temperature value (°C by default) and a small weather condition icon per panel.
- On WiFi failure: an error message is shown, then the board deep sleeps briefly before retrying.
- Serial Monitor: WiFi/NTP connection status and weather fetch retries (if any).

## Notes

- Display uses the Inkplate 2 tri-color palette (`INKPLATE2_BLACK` / `INKPLATE2_WHITE` / `INKPLATE2_RED`) with a full refresh each cycle.
- Weather data is fetched with `display.wifi.downloadFileHTTPS()` and parsed with cJSON (`REQUIRES "json"` in `main/CMakeLists.txt`) instead of ArduinoJson.
- Only the small (48x48, `_S_`) monochrome icon variants from the original Arduino project are ported to `main/binary_Icons/` - the full-size color icon headers were unused by the sketch itself and were dropped to avoid bloating the firmware image (~90 KB each, 18 unused files). See the comment in `main/icons.h`.
- Unlike the original sketch (which retries the weather fetch forever on failure), this port gives up after `MAX_FETCH_RETRIES` attempts per wake cycle so a misconfigured API key/location cannot hang the board awake and drain the battery; it simply tries again on the next scheduled wake-up.
- The original sketch's WiFi-retry deep sleep passed a seconds value where the underlying API expects microseconds, so it actually only slept ~10 ms instead of 10 s before retrying. This port fixes that (`DELAY_WIFI_RETRY_MS`, correctly converted to microseconds).
- Inkplate 2 has no onboard RTC (see `components/inkplate/include/boards/Inkplate2.h`), so time is obtained via `display.wifi.setCurrentTime()` (NTP) rather than an RTC chip, matching what the original sketch already did.
- API rate limits apply - verify your current OpenWeatherMap quota before lowering `DELAY_MS`.
- Icons are selected by matching OpenWeatherMap condition abbreviations (e.g. `"01d"`, `"10n"`) against the built-in icon table.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
