# OpenMeteo Weather Station

Weather dashboard for Soldered Inkplate 2, powered by the free Open-Meteo API (no API key required).

## Overview

Connects Inkplate 2 to WiFi and fetches current conditions plus a 7-day forecast from the [Open-Meteo](https://open-meteo.com/) "forecast" API for a configured latitude/longitude. The dashboard alternates between two layouts on every wake cycle: a 3-day forecast strip (icon + min/max temperature) and a current-conditions summary (city, temperature, condition). After each refresh the board goes into deep sleep and repeats the cycle - because deep sleep resets the ESP32, execution always restarts from `app_main()`, with the layout choice persisted across sleep in RTC memory (`bootCount`).

## Hardware Required

- Soldered Inkplate 2
- USB cable
- Stable WiFi connection with Internet access

## Setup

### 1. Set your location

Open-Meteo needs a latitude/longitude to fetch weather for. Edit `main/main.cpp`:

```cpp
#define LOCATION_LAT 45.5550f
#define LOCATION_LON 18.6955f
```

Optionally also set `MY_CITY` / `MY_USERNAME` (display labels) and `METRIC_UNITS` (`true` for Celsius/km-h, `false` for Fahrenheit/mph).

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

- Display: a weather dashboard that alternates between two layouts on each wake - a 3-day forecast strip (weather icon + min/max temperature per day) on odd wakes, and a current city/temperature/condition summary on even wakes.
- On WiFi failure: a "WiFi connection failed" screen, then the board deep sleeps briefly before retrying.
- On API failure: an "HTTP request failed" screen.
- Serial Monitor: WiFi connection status and weather fetch logs.

## Notes

- Display uses the Inkplate 2 tri-color palette (`INKPLATE2_BLACK` / `INKPLATE2_WHITE` / `INKPLATE2_RED`) with a full refresh each cycle.
- Weather data is fetched with `esp_http_client` and parsed with cJSON (`REQUIRES "json"` in `main/CMakeLists.txt`), following the same technique as `projects/quotables`, instead of the original sketch's HTTPClient + ArduinoJson.
- The original Arduino sketch derived "now" (used to pick the hourly-forecast start index and to name the 7 forecast days) from `configTime()`/NTP using a user-supplied UTC offset. Since the API is requested with `timezone=auto`, every timestamp it returns (`current.time`, `daily.time`, `sunrise`/`sunset`) is already in the requested location's local time - this port reads "now" directly from those fields instead (see `main/Network.cpp`), so no NTP/timezone configuration is needed at all, and the day-of-week for each forecast day is computed from `daily.time[0]` with Sakamoto's algorithm rather than the device's own clock.
- All icon (`main/binary_Icons/`) and font (`main/fonts/`) headers from the original project are ported verbatim, even the handful (`icon_s_moon`, `icon_s_thermometer`, the battery icons, `Gui::getBatteryIcon()`, `Gui::drawTemperaturePrecipGraph()`) that are unused by both dashboard layouts - the original example never implemented or called them either, so they're kept only for structural parity.
- The 7-day/6-hour forecast query only requests the fields `WeatherData` actually stores; the original sketch also requested (but never stored or displayed) `wind_speed_10m_max`, `wind_direction_10m_dominant` and `precipitation_probability_max`.
- Arduino `String` members are replaced with fixed-size `char[]` buffers throughout `Network.cpp`/`WeatherData.h`, avoiding hidden heap allocations.
- API rate limits/availability apply - verify Open-Meteo's status if the dashboard shows the "HTTP request failed" screen repeatedly.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
- Open-Meteo API docs: https://open-meteo.com/en/docs
