/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetch and display events from a public Google Calendar on
 *              Soldered Inkplate 13SPECTRA.
 *
 * @details     Connects Inkplate 13SPECTRA to WiFi, syncs the wall clock via
 *              SNTP, then requests upcoming events from a *public* Google
 *              Calendar using the Google Calendar API (Calendar ID + API
 *              key) and renders a day/agenda view on the 6-color e-paper
 *              screen.
 *
 *              The HTTPS GET request is performed with esp_http_client
 *              (verified against the ESP-IDF certificate bundle, since
 *              googleapis.com is signed by a well-known public CA), and the
 *              JSON response is parsed with cJSON via the NetworkFunctions
 *              helper, which also follows Google's "nextPageToken"
 *              pagination for calendars with many upcoming events. Event
 *              storage (calendarData) and drawing (Gui) are split into their
 *              own files, mirroring the original sketch's src/ layout.
 *
 *              Note: this example (like the original Arduino sketch it was
 *              ported from) authenticates with a simple Google API key
 *              against a *public* calendar - it does not implement the
 *              OAuth2 refresh-token flow required to read a *private*
 *              calendar.
 *
 *              After drawing the calendar, the board deep sleeps for
 *              TIME_TO_SLEEP seconds and wakes up to refresh, matching the
 *              original sketch's refresh cadence.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      WiFi (2.4 GHz) + Internet access, a *public* Google Calendar
 *               ID, and a Google API key with the Calendar API enabled.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - CALENDAR_ID, GOOGLE_API_KEY (below), and TIME_ZONE_OFFSET_HOURS (in
 *   includes.h) -> fill in your own values
 *
 * How to use:
 * 1) Make your calendar public:
 *    Google Calendar -> Settings -> your calendar -> Access permissions ->
 *    enable public access (otherwise API requests may return 404).
 * 2) Get the Calendar ID:
 *    Google Calendar -> Settings -> your calendar -> Integrate calendar ->
 *    copy "Calendar ID" (e.g. ...@group.calendar.google.com).
 * 3) Create an API key and enable the Google Calendar API:
 *    Google Cloud Console -> APIs & Services -> enable "Google Calendar
 *    API", then Credentials -> Create credentials -> API key (otherwise you
 *    may see 403 errors).
 * 4) Fill in CALENDAR_ID and GOOGLE_API_KEY below, and TIME_ZONE_OFFSET_HOURS
 *    in includes.h.
 * 5) Configure WiFi credentials via menuconfig, then build and flash.
 *
 * Expected output:
 * - E-paper: a day/agenda view with a black header bar (current date, day of
 *   week, month/year, "Last Updated" time) followed by upcoming events
 *   grouped by day, with the current event (if any) highlighted in the
 *   configured highlight color.
 * - Serial Monitor: WiFi/time-sync status and HTTP/API error messages.
 *
 * Notes:
 * - Display mode: Inkplate 13SPECTRA's native 6-color e-paper mode (there is
 *   no setDisplayMode() call on this board - it always renders in color).
 * - Orientation: the board defaults to rotation 3 (landscape) right after
 *   construction (see Inkplate::Inkplate() for
 *   CONFIG_INKPLATE_BOARD_INKPLATE13); Gui::showCalendar() then explicitly
 *   switches to rotation 0, matching the original sketch's Gui.cpp, so the
 *   1200x1600 portrait layout coordinates line up with width()/height().
 *   Unlike the Inkplate 6Color port, this main.cpp itself does not call
 *   setRotation() - the original Inkplate13SPECTRA_Google_Calendar.ino
 *   doesn't either, leaving wifiError()/showError() drawn in the default
 *   landscape rotation.
 * - RAM usage: parsing the API response (JSON) can be memory-intensive; keep
 *   CALENDAR_MAX_RESULTS (in Network.cpp) reasonable if you see instability.
 * - Between refreshes the board deep sleeps; execution always restarts from
 *   app_main() on wake, so all state lives in the objects declared here.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "esp_log.h"
#include "esp_sleep.h"
#include "includes.h"

static const char *TAG = "GOOGLE_CALENDAR";

// TODO: fill in your *public* Google Calendar ID (Google Calendar -> Settings
// -> your calendar -> Integrate calendar -> "Calendar ID").
#define CALENDAR_ID "yourpublicgooglecalid@group.calendar.google.com"

// TODO: fill in a Google API key with the Google Calendar API enabled
// (Google Cloud Console -> APIs & Services -> Credentials -> Create
// credentials -> API key).
#define GOOGLE_API_KEY "YOUR_GOOGLE_API_KEY"

// Color used to highlight the currently ongoing event. Inkplate 13SPECTRA
// supports only 6 colors - INKPLATE_BLACK, INKPLATE_WHITE, INKPLATE_YELLOW,
// INKPLATE_RED, INKPLATE_BLUE, INKPLATE_GREEN (see Inkplate13.h) - there is
// no INKPLATE_ORANGE on this board, unlike Inkplate 6Color. INKPLATE_RED is
// used here (matching color index 3 from the original sketch's comment,
// which happens to be the same color the original also uses for the "No
// more events" message) since it reads clearly as a highlight against black
// text on a white background; INKPLATE_YELLOW is a reasonable alternative.
#define HIGHLIGHT_COLOR INKPLATE_RED

#define WIFI_CONNECT_TIMEOUT_MS 30000

// --- Deep Sleep Configuration ---
#define uS_TO_S_FACTOR 1000000ULL // Convert microseconds to seconds
#define TIME_TO_SLEEP 600         // Sleep time: 600 seconds = 10 minutes

extern "C" void app_main(void) {
  Inkplate display;
  calendarData calendar;
  NetworkFunctions network(CALENDAR_ID, GOOGLE_API_KEY);
  Gui gui(display);

  display.begin();        // Start the Inkplate display
  display.clearDisplay(); // Clear the frame buffer

  gui.setHighlightColor(HIGHLIGHT_COLOR);

  // Connect to WiFi using credentials configured via menuconfig (never
  // hardcoded).
  if (display.wifi.begin() != ESP_OK ||
      !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    gui.wifiError();
  } else {
    // Sync the wall clock via SNTP; every local-time computation elsewhere
    // in this example applies TIME_ZONE_OFFSET_HOURS (includes.h) on top of
    // this UTC epoch.
    display.wifi.setCurrentTime();

    if (network.fetchCalendar(&calendar)) {
      ESP_LOGI(TAG, "Calendar loaded.");
      gui.showCalendar(&calendar);
    } else {
      ESP_LOGE(TAG, "Failed to load calendar.");
      gui.showError("Failed to load calendar.");
    }
  }

  // Sleep to save power; wakes every TIME_TO_SLEEP seconds and restarts from
  // app_main() to refresh the calendar.
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
