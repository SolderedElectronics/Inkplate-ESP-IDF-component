# News

Fetch top news headlines from NewsAPI.org over WiFi and render a "World News" layout on Soldered Inkplate 4TEMPERA, refreshing periodically via deep sleep.

## Overview

Connects Inkplate 4TEMPERA to WiFi (credentials configured via menuconfig), synchronizes time via NTP, and fetches the `top-headlines` endpoint from [NewsAPI.org](https://newsapi.org/) through the `NetworkFunctions` helper (`main/Network.cpp` + `main/Network.h`), which performs the HTTPS request with `esp_http_client` (verified against the ESP-IDF certificate bundle) and parses the JSON response with cJSON.

`main.cpp` then renders a simple newspaper-style screen: a "World News" title, the current date and last-update time, and a list of headline/description boxes drawn with `drawTextBox()` using three custom fonts (`FreeSerifItalic24pt7b` for the title, `Inter12pt7b` for the date row and descriptions, `GT_Pressura16pt7b` for the headlines).

After updating the display, the board enters deep sleep and wakes every `REFRESH_INTERVAL_US` (default: 1 hour) to fetch and show fresh headlines. Because deep sleep resets the ESP32, all logic lives in `app_main()`, which runs again from scratch on every wake.

If the WiFi connection fails, an error message is shown and the device deep sleeps briefly before automatically retrying. If the news fetch fails (bad API key, rate limit, network error), a "Failed to fetch news" message is shown instead of the headline list, and the board still waits the full refresh interval before trying again — matching the original Arduino sketch's behavior.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB-C cable
- Stable WiFi connection (2.4 GHz), internet access
- A NewsAPI.org API key

## Setup

### 1. Set your NewsAPI.org API key and timezone

In `main/main.cpp`, fill in:

```cpp
#define NEWS_API_KEY "YourNewsAPIKey"
#define TIMEZONE_OFFSET_HOURS 2
```

Create a free NewsAPI.org account and generate an API key at https://newsapi.org/. `TIMEZONE_OFFSET_HOURS` is the UTC offset for your location (e.g. `2` for UTC+2, `-5` for UTC-5), used to compute the displayed date/last-update time.

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate4**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: "World News" title, current date and last-update time, followed by a list of headline boxes with descriptions (as provided by NewsAPI.org).
- Serial Monitor: WiFi/NTP connection status and network/debug logs from `Network.cpp` (useful for troubleshooting).
- On WiFi failure: an "Unable to connect to WiFi" message is shown, then the board deep sleeps briefly and retries automatically.
- On fetch failure: a "Failed to fetch news" message is shown; the board still deep sleeps for the full `REFRESH_INTERVAL_US` before retrying.
- Every `REFRESH_INTERVAL_US` (default 1 hour), the board wakes, fetches fresh headlines, and updates the display.

## Notes

- Display mode is 1-bit (BW); this example uses a full refresh (`display.display()`) — partial update is not used.
- Deep sleep resets the ESP32 on every wake — there is no `loop()`; all logic runs once per boot inside `app_main()`.
- `display.wifi.setCurrentTime()` sets the system TZ internally, but `time()` always returns UTC seconds regardless of TZ, so this example applies `TIMEZONE_OFFSET_HOURS` manually (same approach as this component's `clock`/`hourly_weather_station` examples, and equivalent to the original sketch's fixed `timeZone` variable).
- `newsapi.org` is signed by a well-known public CA, so `Network.cpp` verifies the server certificate using the ESP-IDF certificate bundle (`esp_crt_bundle_attach`) rather than disabling TLS verification. This requires `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` (already set in `sdkconfig.defaults`) and `REQUIRES "mbedtls"` in `main/CMakeLists.txt`.
- `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` are kept enabled in `sdkconfig.defaults` for parity with the other ported examples, but are not actually relied on here since `esp_http_client_config_t.crt_bundle_attach` is set and the connection is verified against the CA bundle.
- NewsAPI.org's free plan enforces rate limits and restricts requests to a limited set of use cases; if requests fail, check your API key validity and plan limits.
- RAM usage: JSON parsing and multiple custom fonts can consume significant memory. `Network.cpp` caps the parsed article count at `MAX_ARTICLES` (20) to keep memory usage bounded.
- Protect your API key — do not commit a real key to a public repository.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
