# Google Calendar

Fetch and display events from a public Google Calendar on Soldered Inkplate 6 Flick.

## Overview

Connects Inkplate 6 Flick to WiFi, synchronizes the wall clock via SNTP, and requests upcoming events from a **public** Google Calendar using the [Google Calendar API](https://developers.google.com/calendar/api) (`GET /calendars/{calendarId}/events`, authenticated with a plain API key). The JSON response is parsed with cJSON, and the events are rendered as a day/agenda view: a black header bar with the current date/day/month plus a "Last Updated" time, followed by upcoming events grouped by day, with any event currently in progress highlighted.

The HTTPS GET request is performed with `esp_http_client`, verified against the ESP-IDF certificate bundle (`esp_crt_bundle_attach`) since `googleapis.com` is signed by a well-known public CA. Event storage (`calendarData`) and drawing (`Gui`) are split into their own files, mirroring the original `Inkplate6FLICK_Google_Calendar` Arduino sketch's `src/` layout.

**Note on authentication:** this example (like the Arduino sketch it was ported from) authenticates with a simple Google **API key** against a calendar that has been made **public**. It does not implement the OAuth2 refresh-token flow needed to read a *private* calendar — the original sketch never used OAuth2 either, only a Calendar ID + API key pair. See "Setup" below for how to obtain both.

After drawing the calendar, the board deep sleeps for `TIME_TO_SLEEP` seconds (default: 10 minutes) and wakes up to refresh, matching the original sketch's refresh cadence. Because deep sleep resets the ESP32, execution always restarts from `app_main()` on every wake.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- Stable WiFi (2.4 GHz) connection with Internet access
- A Google account with a calendar you're willing to make public, plus a Google API key

## Setup

### 1. Make your calendar public

Google Calendar (web) -> Settings (gear icon) -> click your calendar under "Settings for my calendars" -> **Access permissions for events** -> enable **"Make available to public"**. Without this, API requests return `404 Not Found`.

### 2. Get the Calendar ID

Same settings page -> **Integrate calendar** -> copy the **Calendar ID** (your primary calendar uses your email address; secondary calendars look like `xxxxxxx@group.calendar.google.com`).

### 3. Create a Google API key

[Google Cloud Console](https://console.cloud.google.com/) -> create/select a project -> **APIs & Services -> Library** -> enable **"Google Calendar API"** -> **APIs & Services -> Credentials** -> **Create credentials -> API key**. Without the API enabled, requests return `403 Forbidden`. Optionally restrict the key to the Calendar API for safety.

### 4. Fill in your credentials

In `main/main.cpp`:

```cpp
#define CALENDAR_ID "yourpublicgooglecalid@group.calendar.google.com"
#define GOOGLE_API_KEY "YOUR_GOOGLE_API_KEY"
```

In `main/includes.h`, set your UTC offset (used since this port syncs time via plain SNTP rather than a timezone-aware service):

```cpp
#define TIME_ZONE_OFFSET_HOURS 2 // e.g. 2 for Osijek, -4 for New York City
```

### 5. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6 Flick**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: a black header bar showing the current day-of-month (large), day of week, month/year, and "Last Updated" time, followed by upcoming events for the next 14 days grouped by day (date + short day name), each with a title and start/end time. An event currently in progress is drawn with a rounded highlight box. If there are no more events in the next two weeks, a message says so.
- On WiFi failure: a "WiFi connection failed." screen.
- On a fetch/parse failure: a "Failed to load calendar." error screen.
- Serial Monitor: WiFi/time-sync status and any HTTP/API error messages.
- The board deep sleeps for `TIME_TO_SLEEP` (default 600 s / 10 minutes) between refreshes.

## Notes

- Display mode: 3-bit grayscale (8 shades, 0 = black, 7 = white), matching the original sketch's `INKPLATE_3BIT` mode. `display.setDisplayMode(GRAYSCALE)` is set explicitly in `app_main()`.
- Orientation: `display.setRotation(1)` is called in `app_main()` for portrait mode. Inkplate 6 Flick is natively 1024x758 (landscape), but `Gui.cpp`'s drawing coordinates go up to y≈975 — well past the native 758 px height, and only fit within the panel's 1024 px side once rotated. This matches a hint left in the original `.ino`, which has a commented-out `inkplate.setRotation(1); // Portrait mode` line right after `clearDisplay()`.
- Time handling: `display.wifi.setCurrentTime()` syncs only the UTC epoch via SNTP; this port then applies `TIME_ZONE_OFFSET_HOURS` (in `includes.h`) by hand wherever "local" time is needed (event grouping, the "is this event happening now" highlight, and the header date). There is no automatic DST adjustment — update the offset twice a year if your location observes daylight saving time.
- RAM usage: the JSON response is read into a heap buffer sized from the `Content-Length` header (falling back to 8 KB if absent) and parsed with cJSON. Keep `CALENDAR_MAX_RESULTS`/`CALENDAR_LOOKAHEAD_DAYS` (in `Network.cpp`) reasonable if you see instability with a very busy calendar.
- Event titles longer than `MAX_SUMMARY_LENGTH` (in `Gui.h`, default 26 characters, matching the original sketch) are truncated with "...".
- `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` are kept enabled in `sdkconfig.defaults` for parity with other ported examples, but are not relied on here — `esp_http_client_config_t.crt_bundle_attach` is set, so the connection to `googleapis.com` is verified against the CA bundle (`CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y`).
- Protect your API key — do not commit a real key to a public repository. Google Calendar API usage quotas apply.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
- Google Calendar API reference: https://developers.google.com/calendar/api/v3/reference/events/list
