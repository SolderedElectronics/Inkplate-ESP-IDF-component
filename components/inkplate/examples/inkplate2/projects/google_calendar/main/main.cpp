/**
 **************************************************
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display upcoming Google Calendar events on Soldered
 *              Inkplate 2.
 *
 * @details     Connects Inkplate 2 to WiFi, synchronizes time via NTP
 *              (display.wifi.setCurrentTime()), fetches events from a
 *              *public* Google Calendar using the Google Calendar REST API
 *              (Calendar ID + API key, singleEvents/orderBy/timeMin/timeMax
 *              query, same as the original sketch), and renders them on the
 *              e-paper display using a small GUI helper class.
 *
 *              The workflow:
 *              - Connect to WiFi (credentials from menuconfig)
 *              - Sync the system clock via NTP
 *              - Request calendar data using CALENDAR_ID and API_KEY
 *              - Render events with Gui::showCalendar()
 *              - Wait REFRESH_INTERVAL_MS and repeat
 *
 *              The original Arduino sketch put the ESP32 into deep sleep
 *              after each update (waking every TIME_TO_SLEEP seconds,
 *              restarting from setup() on each wake). This port keeps the
 *              board powered and instead loops inside app_main() with a
 *              vTaskDelay() of the same interval, matching the pattern used
 *              by this component's other ported "projects" examples.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi connection + Internet access, a *public* Google
 *               Calendar, a Google Cloud API key with the Calendar API
 *               enabled
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - CALENDAR_ID, API_KEY, TIMEZONE_OFFSET_HOURS below -> fill in your own
 *   values (see README.md for how to obtain a Calendar ID and API key)
 *
 * How to use:
 * 1) Make your Google Calendar public and grab its Calendar ID.
 * 2) Enable the Google Calendar API in Google Cloud Console and generate an
 *    API key.
 * 3) Set CALENDAR_ID, API_KEY and TIMEZONE_OFFSET_HOURS below.
 * 4) Configure WiFi credentials in menuconfig.
 * 5) Build and flash to Inkplate 2.
 * 6) The device connects, syncs time, fetches events, displays them, waits
 *    REFRESH_INTERVAL_MS, then repeats.
 *
 * Expected output:
 * - Display: agenda-style list of upcoming events for the next 14 days,
 *   grouped by day.
 * - On WiFi error: dedicated WiFi error screen, retried automatically.
 * - On API error: error message shown via GUI (see Notes for common causes).
 *
 * Notes:
 * - The original sketch authenticates with a plain Google Cloud API key
 *   against a *public* calendar (Calendar ID + "key=" query parameter), not
 *   OAuth2 - that is exactly what this port keeps (see Network.cpp). If you
 *   need a private/personal calendar, you would have to add an OAuth2
 *   token-refresh flow yourself; that is out of scope for this port because
 *   the original example never implemented one.
 * - Display mode is 1-bit (BW). A full refresh is used when rendering the
 *   GUI (display.display()).
 * - Google API errors:
 *   - 403 Forbidden -> Calendar API not enabled on the Google Cloud project.
 *   - 404 Not Found -> calendar not public, or wrong Calendar ID.
 * - Protect your API key: do not commit a real key to a public repository.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 **************************************************/

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "includes.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <ctime>

static const char *TAG = "GOOGLE_CALENDAR";

// --- Google Calendar configuration ---

// TODO: fill in your public Google Calendar's ID. Find it under Google
// Calendar -> Settings -> (select your calendar) -> "Integrate calendar" ->
// Calendar ID. Looks like "abc123@group.calendar.google.com".
#define CALENDAR_ID "yourpublicgooglecalid@group.calendar.google.com"

// TODO: fill in a Google Cloud API key with the "Google Calendar API"
// enabled. Create one in Google Cloud Console -> APIs & Services ->
// Credentials -> Create Credentials -> API key. See README.md for the full
// setup steps.
#define API_KEY "yourapikey"

// UTC offset (in hours) for your local timezone, e.g. 2 for Osijek (UTC+2),
// -4 for New York City (UTC-4).
#define TIMEZONE_OFFSET_HOURS 2

// How often to re-fetch and redraw the calendar, in milliseconds. Matches
// the original sketch's 10-minute TIME_TO_SLEEP deep-sleep interval.
#define REFRESH_INTERVAL_MS (600000) // 10 minutes

// How long to wait for a WiFi connection before giving up, in milliseconds.
#define WIFI_CONNECT_TIMEOUT_MS 30000

// Delay before retrying after a WiFi connection failure, in milliseconds.
#define WIFI_RETRY_INTERVAL_MS (30000) // 30 seconds

extern "C" void app_main(void) {
  Inkplate display;
  calendarData calendar;
  NetworkFunctions network(CALENDAR_ID, API_KEY);
  Gui gui(display);

  display.clearDisplay();
  display.setRotation(1); // Portrait mode; use setRotation(3) if upside down.

  while (true) {
    // Connect to WiFi using credentials configured via menuconfig.
    ESP_LOGI(TAG, "Connecting to WiFi...");
    if (display.wifi.begin() != ESP_OK ||
        !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
      ESP_LOGE(TAG, "WiFi connection failed");
      gui.wifiError();
      vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_INTERVAL_MS));
      continue;
    }

    ESP_LOGI(TAG, "WiFi connected, syncing time...");
    display.wifi.setCurrentTime();

    ESP_LOGI(TAG, "Fetching calendar events...");
    if (network.fetchCalendar(&calendar, TIMEZONE_OFFSET_HOURS)) {
      ESP_LOGI(TAG, "Calendar loaded (%d events)", calendar.getEventCount());

      // time() always returns UTC seconds; apply the same manual offset
      // used inside fetchCalendar() so the GUI's "current date" fallback
      // matches what was just fetched.
      time_t nowLocal = time(nullptr) + (time_t)TIMEZONE_OFFSET_HOURS * 3600;
      struct tm nowTm;
      gmtime_r(&nowLocal, &nowTm);

      gui.showCalendar(&calendar, nowTm);
    } else {
      ESP_LOGE(TAG, "Failed to load calendar");
      gui.showError("Failed to load calendar.");
    }

    vTaskDelay(pdMS_TO_TICKS(REFRESH_INTERVAL_MS));
  }
}
