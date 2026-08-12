/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetch news headlines from NewsAPI.org over WiFi, render a
 *              "World News" layout, then deep sleep between updates.
 *
 * @details     Connects the Inkplate 13SPECTRA to WiFi (credentials
 *              configured via menuconfig), synchronizes time via NTP, and
 *              fetches top headlines from NewsAPI.org through
 *              NetworkFunctions (Network.cpp/h - esp_http_client + cJSON).
 *              The response is parsed into a heap-allocated array of `news`
 *              items (title + description), and this file renders a simple
 *              newspaper-style screen: a "World News" title, the current
 *              date/last-update time, and a list of headline/description
 *              boxes drawn with drawTextBox() using two custom fonts.
 *
 *              After updating the e-paper display, the ESP32 enters deep
 *              sleep for REFRESH_INTERVAL_US. When it wakes, execution
 *              restarts from app_main(), fetches fresh news, and redraws the
 *              screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      WiFi (2.4 GHz) connection + Internet access, NewsAPI.org API
 *               key
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13 (used for both Inkplate 13
 *   and Inkplate 13SPECTRA)
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - main.cpp   -> NEWS_API_KEY: your NewsAPI.org API key
 * - main.cpp   -> TIMEZONE_OFFSET_HOURS: UTC offset for your location
 *
 * How to use:
 * 1) Create a NewsAPI.org account and generate an API key
 *    (https://newsapi.org/).
 * 2) Set NEWS_API_KEY and TIMEZONE_OFFSET_HOURS below.
 * 3) Configure WiFi credentials via menuconfig.
 * 4) Build and flash to Inkplate 13SPECTRA.
 * 5) The device fetches news, renders the page once, then deep sleeps. It
 *    wakes and refreshes every REFRESH_INTERVAL_US.
 *
 * Expected output:
 * - Display: "World News" title, current date and last-update time, followed
 *   by a list of headline boxes with descriptions (as provided by
 *   NewsAPI.org).
 * - Serial Monitor: WiFi/NTP status and network/debug output from
 *   Network.cpp (useful for troubleshooting).
 * - On WiFi failure: an error message is shown and the board deep sleeps
 *   briefly before automatically retrying.
 * - On fetch failure: a "Failed to fetch news" message is shown instead of
 *   the headline list, and the board still deep sleeps for the full
 *   REFRESH_INTERVAL_US (matching the original sketch's behavior - it does
 *   not shorten the wait on a failed fetch, only on a failed WiFi connect).
 *
 * Notes:
 * - Display mode: Inkplate 13SPECTRA is a 6-color e-paper board (black,
 *   white, yellow, red, blue, green via the INKPLATE_* macros in
 *   Inkplate13.h). Unlike Inkplate 6Color, there is no INKPLATE_ORANGE on
 *   this board, and there is no setDisplayMode() call - it always renders in
 *   its native color mode. This example only draws with
 *   INKPLATE_BLACK/INKPLATE_WHITE, matching the original sketch's layout.
 * - Orientation: Inkplate::Inkplate() already calls setRotation(3) by default
 *   for CONFIG_INKPLATE_BOARD_INKPLATE13, so unlike the original
 *   Inkplate13SPECTRA_News.ino (which calls inkplate.setRotation(3) again
 *   inside drawNews()) and unlike this component's Inkplate 6Color port
 *   (whose board defaults to rotation 2, so an explicit setRotation(3) is
 *   actually needed there), this file does not call setRotation() at all -
 *   it would just re-select the rotation the display already starts in.
 *   All layout coordinates below are computed from display.width()/height(),
 *   so they still adapt automatically to the resulting canvas size.
 * - The layout is drawn once per wake and pushed with a full refresh
 *   (display.display()); Inkplate 13SPECTRA does not support partial
 *   updates.
 * - Deep sleep restarts the ESP32; all logic lives in app_main() (no loop()).
 * - display.wifi.setCurrentTime() sets the system TZ internally, but time()
 *   always returns UTC seconds regardless of TZ, so TIMEZONE_OFFSET_HOURS is
 *   applied manually here (same approach as this component's Inkplate 6 and
 *   Inkplate 6Color news examples, and equivalent to the original sketch's
 *   fixed `timeZone` variable).
 * - API/network limits: NewsAPI.org enforces rate limits and plan
 *   restrictions. If requests fail, check API key validity, plan limits, and
 *   WiFi stability.
 * - RAM usage: JSON parsing and multiple custom fonts can consume significant
 *   memory. If you experience instability, reduce MAX_ARTICLES in
 *   Network.cpp or simplify the layout.
 * - newsapi.org is signed by a well-known public CA, so Network.cpp verifies
 *   the server certificate using the ESP-IDF certificate bundle
 *   (esp_crt_bundle_attach) rather than disabling TLS verification.
 * - Protect your API key: do not commit a real key to a public repository.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "Network.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Custom fonts used for the title, date row, headlines, and descriptions.
#include "fonts/FreeSerifBold12pt7b.h"
#include "fonts/FreeSerifItalic24pt7b.h"
#include "fonts/Inter10pt7b.h"

static const char *TAG = "NEWS";

//---------- CHANGE HERE -------------:

// TODO: fill in your NewsAPI.org API key (https://newsapi.org/)
#define NEWS_API_KEY "YourNewsAPIKey"

// TODO: fill in your local timezone as a UTC offset in hours
// (e.g. 2 for UTC+2, -5 for UTC-5). WiFi SSID/password are configured
// separately via "idf.py menuconfig" -> WiFi Configuration.
#define TIMEZONE_OFFSET_HOURS 2

//-------------------------------------

// Delay between news refreshes (deep sleep), in microseconds. Matches the
// original sketch's DELAY_MS (1 hour).
#define REFRESH_INTERVAL_US (60ULL * 60ULL * 1000000ULL)

// How long to wait for a WiFi connection before giving up.
#define WIFI_CONNECT_TIMEOUT_MS 15000

// How long to deep sleep before retrying after a failed WiFi connection.
// Matches the original sketch's DELAY_WIFI_RETRY_SECONDS.
#define WIFI_RETRY_INTERVAL_US (10ULL * 1000000ULL)

/**
 * Draws the "World News" layout: title, dividing lines, current date/last
 * update time, and up to as many headline/description boxes as fit on the
 * screen. Mirrors the original sketch's drawNews().
 */
static void drawNews(Inkplate &display, news *entities, int count) {
  // No setRotation() call here - the board already defaults to rotation 3
  // (see the file header's Orientation note).
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);

  // Title: "World News", roughly centered (same crude width estimate the
  // original sketch used instead of getTextBounds()).
  display.setFont(&FreeSerifItalic24pt7b);
  int textWidth = strlen("World News") * 10;
  int centerX = (display.width() - textWidth) / 2;
  display.setCursor(centerX - 60, 40);
  display.print("World News");

  // Dividing line below the title.
  int xStart = display.width() * 0.05;
  int xEnd = display.width() * 0.95;
  for (int lineY = 60; lineY < 63; lineY++) {
    display.drawLine(xStart, lineY, xEnd, lineY, INKPLATE_BLACK);
  }

  // Current date/time. time() returns UTC seconds regardless of the TZ set
  // by setCurrentTime(), so the local offset is applied manually (see Notes
  // in the file header).
  time_t nowUtc = time(nullptr) + (time_t)TIMEZONE_OFFSET_HOURS * 3600;
  struct tm timeInfo;
  gmtime_r(&nowUtc, &timeInfo);

  char dateStr[48];
  char updateStr[48];
  snprintf(dateStr, sizeof(dateStr), "Date : %02d.%02d.%04d", timeInfo.tm_mday,
           timeInfo.tm_mon + 1, timeInfo.tm_year + 1900);
  snprintf(updateStr, sizeof(updateStr), "Last update : %02d:%02d",
           timeInfo.tm_hour, timeInfo.tm_min);

  display.setFont(&Inter10pt7b);
  int yPos = 83;

  // Date, left-aligned.
  display.setCursor(20, yPos);
  display.print(dateStr);

  // "Last update", right-aligned (width estimated from average char width,
  // same as the original sketch).
  int updateStrWidth = strlen(updateStr) * 12;
  int xRight = display.width() - updateStrWidth + 25;
  display.setCursor(xRight, yPos);
  display.print(updateStr);

  // Dividing line below the date row.
  for (int lineY = 93; lineY < 96; lineY++) {
    display.drawLine(xStart, lineY, xEnd, lineY, INKPLATE_BLACK);
  }

  // Headline boxes.
  int startY = 140;
  int boxHeight = 120;
  int boxSpacing = 10;
  int leftMargin = 10;
  int rightMargin = 90;
  int maxBoxes = (display.height() - startY) / (boxHeight + boxSpacing);

  for (int i = 0; i < maxBoxes && i < count; i++) {
    // Stop rendering further boxes once an item is missing a title or
    // description, same as the original sketch's loop condition.
    if (entities[i].title == nullptr || entities[i].description == nullptr)
      break;

    int y0 = startY + i * (boxHeight + boxSpacing);
    int y1 = y0 + boxHeight;

    display.drawTextBox(leftMargin, y0, display.width() - rightMargin,
                        y0 + 70, entities[i].title, 1, &FreeSerifBold12pt7b,
                        26, false, 12);

    display.drawTextBox(leftMargin, y0 + 65, display.width() - rightMargin - 25,
                        y1, entities[i].description, 1, &Inter10pt7b, 20,
                        false, 10);
  }
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextWrap(false);
  display.clearDisplay();
  display.display();

  // Connect to WiFi using credentials configured via menuconfig - never
  // hardcode credentials here.
  ESP_LOGI(TAG, "Connecting to WiFi...");
  if (display.wifi.begin() != ESP_OK ||
      !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
    ESP_LOGE(TAG, "WiFi connection failed");

    display.clearDisplay();
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
    display.setTextSize(2);
    display.setCursor(50, 230);
    display.print("Unable to connect to WiFi.\nPlease check SSID and "
                  "password\nin menuconfig!");
    display.display();

    // Go back to sleep for a short while, then retry.
    esp_sleep_enable_timer_wakeup(WIFI_RETRY_INTERVAL_US);
    esp_deep_sleep_start();
    return; // Never reached, app_main() restarts on wake.
  }
  ESP_LOGI(TAG, "WiFi connected");

  // Synchronize time via NTP (blocks until synced), matches the original
  // sketch's setTime().
  display.wifi.setCurrentTime();

  // Fetch news data via the already-ported Network module.
  NetworkFunctions network;
  network.setApiKey(NEWS_API_KEY);

  int count = 0;
  news *entities = network.getData(display, &count);

  display.clearDisplay();

  if (entities != nullptr) {
    drawNews(display, entities, count);
  } else {
    // Matches the original sketch's fallback message on fetch failure.
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
    display.setCursor(50, 230);
    display.setTextSize(2);
    display.print("Failed to fetch news");
  }

  display.display();

  delete[] entities;

  ESP_LOGI(TAG, "News displayed, entering deep sleep");

  // Go to sleep until the next scheduled refresh. Matches the original
  // sketch: even on a fetch failure, it waits the full interval rather than
  // retrying sooner.
  esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
}
