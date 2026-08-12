# Google Calendar

Display upcoming events from a public Google Calendar on Soldered Inkplate 2, refreshing periodically.

## Overview

Connects Inkplate 2 to WiFi, syncs the system clock over NTP, and fetches events for the next 14 days from a *public* Google Calendar using the Google Calendar REST API (`events.list`, authenticated with a Google Cloud API key). Events are parsed with cJSON and rendered as an agenda list grouped by day, using a small `Gui` helper class.

The HTTP GET is built with `esp_http_client` (verified against the ESP-IDF certificate bundle, since `googleapis.com` is signed by a well-known public CA), and the JSON response is parsed with cJSON — replacing the original sketch's `HTTPClient` + `ArduinoJson`.

After updating the display, the board waits `REFRESH_INTERVAL_MS` (default: 10 minutes, matching the original sketch's `TIME_TO_SLEEP`) and fetches/redraws again. The original Arduino sketch used deep sleep for this refresh cycle (waking and restarting from `setup()` every `TIME_TO_SLEEP` seconds); this port keeps the board powered and loops inside `app_main()` instead, matching the pattern used by this component's other ported "projects" examples.

If the WiFi connection fails, an error screen is shown and the board retries automatically.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- Stable WiFi connection with Internet access

## Setup

### 1. Make your Google Calendar public and get its Calendar ID

1. Open [Google Calendar](https://calendar.google.com/) and go to **Settings** for the calendar you want to display (hover over it under "My calendars" -> the three-dot menu -> **Settings and sharing**).
2. Under **Access permissions for events**, enable **"Make available to public"**. (This example uses a simple API key against a public calendar — it does not implement an OAuth2 refresh-token flow, since the original Arduino sketch didn't either. If you need a private/personal calendar, you would need to add OAuth2 support yourself.)
3. Scroll down to **Integrate calendar** and copy the **Calendar ID** (looks like `abc123def456@group.calendar.google.com`, or your email address if it's your primary calendar).

### 2. Enable the Calendar API and create an API key

1. Open the [Google Cloud Console](https://console.cloud.google.com/) and select (or create) a project.
2. Go to **APIs & Services -> Library**, search for **Google Calendar API**, and click **Enable**.
3. Go to **APIs & Services -> Credentials -> Create Credentials -> API key**.
4. (Recommended) Restrict the key to the Google Calendar API only.
5. Copy the generated API key.

### 3. Configure the example

In `main/main.cpp`, fill in:

```cpp
#define CALENDAR_ID "yourpublicgooglecalid@group.calendar.google.com"
#define API_KEY "yourapikey"
#define TIMEZONE_OFFSET_HOURS 2 // UTC+2 for Osijek, UTC-4 for New York City, etc.
```

### 4. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate2**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: an agenda-style list of upcoming events for the next 14 days, grouped by day (date + short day name on the left, event title + start/end time on the right). A "No more events in the next 2 weeks!" message is shown once all events fit on screen with room to spare.
- Serial Monitor: WiFi/NTP/fetch status logs.
- On WiFi failure: a "WiFi connection failed" screen, retried automatically every `WIFI_RETRY_INTERVAL_MS`.
- On API failure: an "Error: Failed to load calendar." message (see Notes below for common causes).
- Every `REFRESH_INTERVAL_MS` (default 10 minutes), the board re-fetches and redraws.

## Notes

- Display mode is 1-bit (BW), with the small red plane also used (`INKPLATE2_RED`) for the event end time. A full refresh is used (`display.display()`).
- Google Calendar API errors:
  - `403 Forbidden` -> the Calendar API is not enabled on your Google Cloud project.
  - `404 Not Found` -> the calendar is not public, or the Calendar ID is wrong.
- The system clock is synced once per refresh cycle via `display.wifi.setCurrentTime()` (NTP). Because `time()` always returns UTC seconds regardless of that call's internal TZ setting, `TIMEZONE_OFFSET_HOURS` is applied manually (same fixed-offset approach as the original sketch's `timeZone` variable) — daylight saving time is not handled automatically.
- This example authenticates with a plain API key against a *public* calendar, exactly like the original Arduino sketch. It does not implement Google's OAuth2 refresh-token flow, so it cannot access a private calendar.
- Protect your API key — do not commit a real key to a public repository. Google Cloud API usage quotas apply.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
