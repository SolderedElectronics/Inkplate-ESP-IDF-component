# Open-Meteo Weather Station

Weather dashboard for Soldered Inkplate 6, fetching current/daily/hourly forecast data from the free [Open-Meteo](https://open-meteo.com/) API and refreshing periodically via deep sleep.

## Overview

Connects Inkplate 6 to WiFi, synchronizes local time via NTP using a configured UTC offset, and performs an HTTPS GET against the Open-Meteo forecast API (no API key required) for a fixed latitude/longitude. The JSON response is parsed with cJSON (verified against the ESP-IDF certificate bundle) into a `WeatherData` struct, and a small `Gui` helper layer renders a dashboard: current conditions with a weather icon, a battery/status panel, an "additional info" panel (feels-like, sunrise/sunset, UV index, wind, precipitation), a 7-day forecast strip, a day/night indicator, and an hourly temperature/precipitation graph — all in 3-bit grayscale.

After drawing the UI, the board enters deep sleep and wakes every `REFRESH_INTERVAL_US` (default: 30 minutes, matching the original sketch's `TIME_TO_SLEEP`) to fetch fresh data and redraw the dashboard. Because deep sleep resets the ESP32, all logic lives in `app_main()`, which runs again from scratch on every wake. If the WiFi connection fails, a "WiFi connection failed" screen is shown instead of the dashboard. If the Open-Meteo request or JSON parsing fails, an "HTTP request failed" screen is shown instead. Either way, the board still sleeps for the same `REFRESH_INTERVAL_US` before the next attempt, matching the original sketch's single, unconditional `esp_deep_sleep_start()` call at the end of `setup()`.

This example ports the Arduino `Inkplate6_OpenMeteo_Weather_Station` sketch's `src/Network.cpp`, `src/Gui.cpp` and `src/WeatherData.cpp` module split to ESP-IDF (`main/Network.cpp`, `main/Gui.cpp`, `main/WeatherData.cpp`), replacing HTTPClient + ArduinoJson with esp_http_client + cJSON, and Arduino `String` fields with fixed-size char buffers.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- Stable 2.4 GHz WiFi connection with Internet access

## Setup

### 1. Select the board and enter WiFi credentials

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6**
- **WiFi Configuration → Enter your SSID and password**

### 2. Set your location

Open `main/main.cpp` and edit the `#define`s near the top:

```c
#define LATITUDE 45.5550f          // Your location's latitude
#define LONGITUDE 18.6955f         // Your location's longitude
#define UTC_OFFSET_HOURS 2         // UTC offset in hours (e.g. 2 for UTC+2, -4 for UTC-4)
#define USE_METRIC_UNITS true      // false = Fahrenheit / mph

#define MY_USERNAME "Username"     // Display-only name shown on screen
#define MY_CITY "Osijek"           // Display-only city label shown on screen
```

`UTC_OFFSET_HOURS` is a fixed offset (no automatic daylight-saving adjustment); update it manually if your region observes DST.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: a weather dashboard with the city label and current temperature/condition icon at the top-left, a battery icon/percentage and "last updated" info at the top-right, an additional-info panel (feels-like, sunrise, sunset, UV index, wind, precipitation) on the left, an hourly temperature/precipitation graph in the center, a day/night indicator box, and a 7-day forecast strip with min/max temperatures and icons along the bottom.
- Serial Monitor: WiFi connection status, NTP sync progress, and weather-fetch logs.
- On WiFi failure: a "WiFi connection failed." screen is shown instead of the dashboard.
- On API failure: an "HTTP request failed." screen is shown instead of the dashboard.
- Every `REFRESH_INTERVAL_US` (default 30 minutes), the board wakes, fetches fresh weather data, and redraws the dashboard.

## Notes

- Display mode is 3-bit grayscale (8 levels, `GRAYSCALE`); partial update is not available in grayscale mode, so this example always performs a full refresh (`display.display()`).
- **Orientation (no rotation, unlike this board's `google_calendar`/`news` ports):** `app_main()` deliberately does **not** call `display.setRotation(...)`. The original Arduino sketch never rotates the display either, and `Gui.cpp`'s layout coordinates confirm it: several `fillRect`/`drawLine` calls reach `x = 800`, which only fits the native 800x600 **landscape** canvas — rotating to the 600x800 portrait orientation used by the calendar/news ports would instead clip most of the dashboard's width. One quirk carried over unmodified from the original sketch: the day/night indicator box in `Gui::displayWeatherData()` is drawn at `y = 645`–`795`, which exceeds the panel's native height of 600 px and is therefore silently clipped (invisible) on real hardware. This is a pre-existing layout bug in the upstream Arduino example, not something introduced by this port, so `Gui.cpp` was kept unchanged to match it verbatim.
- Deep sleep resets the ESP32 on every wake — there is no `loop()`; all logic runs once per boot inside `app_main()`.
- The Open-Meteo API is public and requires no key; its response format may change over time — update `Network.cpp` if parsing starts failing.
- `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` is enabled in `sdkconfig.defaults` so the HTTPS request to `api.open-meteo.com` is verified against the standard ESP-IDF certificate bundle rather than skipping TLS verification.
- Update period is configured via `REFRESH_INTERVAL_US` (in `main/main.cpp`, in microseconds).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Open-Meteo API docs: https://open-meteo.com/en/docs
- Image tool: https://tools.soldered.com/tools/image-converter/
