# Open-Meteo Weather Station

Weather dashboard for Soldered Inkplate 13SPECTRA, fetching current/daily/hourly forecast data from the free [Open-Meteo](https://open-meteo.com/) API and refreshing periodically via deep sleep.

## Overview

Connects Inkplate 13SPECTRA to WiFi, synchronizes local time via NTP using a configured UTC offset, and performs an HTTPS GET against the Open-Meteo forecast API (no API key required) for a fixed latitude/longitude. The JSON response is parsed with cJSON (verified against the ESP-IDF certificate bundle) into a `WeatherData` struct, and a small `Gui` helper layer renders a dashboard: current conditions with a weather icon, a battery/status panel, an additional-info panel (feels like, sunrise/sunset, UV index, wind, precipitation), a day/night indicator, a 7-day forecast strip with min/max temperature indicators, and an hourly temperature/precipitation graph.

After drawing the UI, the board enters deep sleep and wakes every `REFRESH_INTERVAL_US` (default: 30 minutes, matching the original sketch's `TIME_TO_SLEEP`) to fetch fresh data and redraw the dashboard. Because deep sleep resets the ESP32, all logic lives in `app_main()`, which runs again from scratch on every wake. If the WiFi connection fails, a "WiFi connection failed" screen is shown instead of the dashboard. If the Open-Meteo request or JSON parsing fails, an "HTTP request failed" screen is shown instead. Either way, the board still sleeps for the same `REFRESH_INTERVAL_US` before the next attempt, matching the original sketch's single, unconditional `esp_deep_sleep_start()` call at the end of `setup()`.

This example ports the Arduino `Inkplate13SPECTRA_OpenMeteo_Weather_Station` sketch's `src/NetworkFunctions.cpp`, `src/Gui.cpp` and `src/WeatherData.cpp` module split to ESP-IDF (`main/Network.cpp`, `main/Gui.cpp`, `main/WeatherData.cpp`), replacing HTTPClient + ArduinoJson with esp_http_client + cJSON, and Arduino `String` fields with fixed-size char buffers. The layout in `main/Gui.cpp` is ported directly from this board's own Arduino sketch (which already targets its native 1600x1200 landscape canvas), not rescaled from the smaller Inkplate 6Color version.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable 2.4 GHz WiFi connection with Internet access

## Setup

### 1. Select the board and enter WiFi credentials

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13**
- **WiFi Configuration → Enter your SSID and password**

### 2. Set your location

Open `main/main.cpp` and edit the `#define`s near the top:

```c
#define LATITUDE 45.5550f          // Your location's latitude
#define LONGITUDE 18.6955f         // Your location's longitude
#define UTC_OFFSET_HOURS 1         // UTC offset in hours (e.g. 1 for UTC+1, -4 for UTC-4)
#define USE_METRIC_UNITS true      // false = Fahrenheit / mph

#define MY_USERNAME "Soldered"     // Display-only name shown on screen
#define MY_CITY "Osijek"           // Display-only city label shown on screen
```

`UTC_OFFSET_HOURS` is a fixed offset (no automatic daylight-saving adjustment); update it manually if your region observes DST.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: a weather dashboard with the city label and current temperature/condition icon at the top-left, a black battery/status panel with white icon and text (percentage, last-updated date/time, username) at the top-right, a black "additional info" panel (feels like, sunrise, sunset, UV index, wind, precipitation) on the left, a day/night indicator, an hourly temperature/precipitation graph, and a black 7-day forecast strip with min/max temperatures and icons along the bottom.
- Serial Monitor: WiFi connection status, NTP sync progress, and weather-fetch logs.
- On WiFi failure: a "WiFi connection failed." screen is shown instead of the dashboard.
- On API failure: an "HTTP request failed." screen is shown instead of the dashboard.
- Every `REFRESH_INTERVAL_US` (default 30 minutes), the board wakes, fetches fresh weather data, and redraws the dashboard.

## Notes

- **Color usage:** Inkplate 13SPECTRA is a 6-color e-paper panel (1600x1200 landscape) supporting black/white/yellow/red/blue/green -- unlike Inkplate 6Color, there is **no orange** on this board. There is no grayscale mode and no `setDisplayMode()` call. `Gui.cpp` draws black text/icons on the cleared white background, black status/additional-info/weekly-forecast panels with white text/icons/arrows on top of them, and a red ("hot"/max) vs. blue ("cold"/min) accent in the hourly temperature graph's labels and precipitation bars. Yellow and green are not used anywhere in this dashboard, matching the original Arduino sketch. Partial update is not available on this board either, so this example always performs a full refresh (`display.display()`).
- The weekly forecast strip shows all 7 fetched days (unlike the Inkplate 6Color port, which only fits 6 across its much smaller panel) -- Inkplate 13SPECTRA's 1600px-wide canvas has room for all of them.
- Deep sleep resets the ESP32 on every wake — there is no `loop()`; all logic runs once per boot inside `app_main()`.
- The Open-Meteo API is public and requires no key; its response format may change over time — update `Network.cpp` if parsing starts failing.
- `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` is enabled in `sdkconfig.defaults` so the HTTPS request to `api.open-meteo.com` is verified against the standard ESP-IDF certificate bundle rather than skipping TLS verification.
- Update period is configured via `REFRESH_INTERVAL_US` (in `main/main.cpp`, in microseconds).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Open-Meteo API docs: https://open-meteo.com/en/docs
- Image tool: https://tools.soldered.com/tools/image-converter/
