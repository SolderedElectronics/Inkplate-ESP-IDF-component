/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetch a random quote from a public API and display it on
 *              Soldered Inkplate 13SPECTRA, then deep sleep and refresh
 *              periodically.
 *
 * @details     This example demonstrates calling a simple public REST API
 *              that returns JSON (no authentication required), extracting
 *              the quote text and author using cJSON via a QuotablesNetwork
 *              helper, and displaying the result on the 6-color e-paper
 *              screen of Inkplate 13SPECTRA.
 *
 *              The quote is rendered inside a text box using drawTextBox()
 *              with a monospace font, and the author is printed in the
 *              lower-right corner. After updating the display, the ESP32
 *              enters deep sleep and wakes every REFRESH_INTERVAL_US
 *              microseconds (default: 5 minutes) to fetch and show a new
 *              quote.
 *
 *              If the WiFi connection fails, an error message is shown and
 *              the device sleeps briefly before retrying. Because deep sleep
 *              resets the ESP32, execution always restarts from app_main()
 *              on each wake.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      Stable WiFi (2.4 GHz) connection + Internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Build and flash to Inkplate 13SPECTRA.
 * 3) On every boot/wake, the device fetches a quote and updates the display,
 *    then deep sleeps for the configured interval.
 *
 * Expected output:
 * - Display: the fetched quote inside a text box using a monospace font in
 *   black on white; the author printed at the bottom-right, prefixed with
 *   "-".
 * - Serial Monitor: connection status and fetch retry logs.
 * - On WiFi failure: an "Unable to connect" message, then a short sleep and
 *   automatic retry.
 *
 * Notes:
 * - Display mode: Inkplate 13SPECTRA is a 6-color e-paper board (black,
 *   white, yellow, red, blue, green via the INKPLATE_* color macros - see
 *   Inkplate13.h). There is no setDisplayMode() call on this board - it
 *   always renders in its native color mode. There is also no
 *   INKPLATE_ORANGE on this board, unlike Inkplate 6Color.
 * - Orientation: the board defaults to rotation 3 (landscape) right after
 *   construction (see Inkplate::Inkplate() for
 *   CONFIG_INKPLATE_BOARD_INKPLATE13). The original
 *   Inkplate13SPECTRA_Quotables.ino doesn't call setRotation() either, so
 *   this port doesn't add one - the layout below is tuned for the resulting
 *   1600x1200 landscape width()/height().
 * - Deep sleep resets the ESP32; all logic lives in app_main().
 * - drawTextBox() truncates with "..." if the quote exceeds the box height.
 * - API behavior/format may change over time; if parsing fails, update
 *   QuotablesNetwork.cpp accordingly.
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

#include "Inkplate.h"
#include "QuotablesNetwork.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fonts/FreeMonoBold12pt7b.h"

static const char *TAG = "QUOTABLES";

// Delay between quote refreshes, in microseconds (5 minutes), matching the
// original sketch's DELAY_S.
#define REFRESH_INTERVAL_US (300ULL * 1000000ULL)
// Delay before retrying after a WiFi connection failure, in microseconds,
// matching the original sketch's DELAY_WIFI_RETRY_SECONDS.
#define WIFI_RETRY_INTERVAL_US (5ULL * 1000000ULL)
// Delay between failed quote-fetch attempts, in milliseconds.
#define FETCH_RETRY_DELAY_MS 1000

// Quote text box layout, ported as-is from the original
// Inkplate13SPECTRA_Quotables.ino, which was already tuned for this board's
// own 1600x1200 landscape panel (unlike the Inkplate 6 sketch reused
// verbatim by other quotables ports, no rescaling is needed here).
#define BOX_LEFT_MARGIN_PX 48    // box left = this
#define BOX_RIGHT_MARGIN_PX 48   // box right = width() - this
#define BOX_TOP_OFFSET_PX 36     // box top = height()/2 - this
#define BOX_BOTTOM_OFFSET_PX 400 // box bottom = height()/2 + this
#define BOX_TEXT_SIZE_MULTIPLIER 2 // text size multiplier passed to drawTextBox()
#define BOX_LINE_SPACING_PX 36   // vertical spacing between wrapped lines
#define BOX_WRAP_FONT_SIZE 24    // font size (pt) used for word-wrap estimate

// Author signature layout, also ported as-is from the original sketch.
#define AUTHOR_RIGHT_MARGIN_PX 50 // right inset for the right-aligned author
#define AUTHOR_BOTTOM_MARGIN_PX 30 // bottom inset for the author baseline

// Buffers used to hold the fetched quote and author name.
static char quote[128];
static char author[64];

// `display` is NOT a global: it's constructed as a local in app_main(). A
// file-scope `Inkplate display;` would race the library's own global
// I2C/PCAL peripheral objects (in BoardCommon.cpp) - C++ leaves
// cross-translation-unit static init order unspecified, so the Inkplate ctor
// can run before the I2C bus/expander objects it depends on, leaving
// peripherals uninitialized.
extern "C" void app_main(void) {
  Inkplate display;
  NetworkFunctions network;

  display.setTextColor(INKPLATE_BLACK);
  // Word wrap is left off: drawTextBox() below manually wraps/paginates the
  // quote itself, and the WiFi-error and author lines are each a single
  // manually-positioned line.
  display.setTextWrap(false);
  display.clearDisplay();
  display.display();

  // Connect to WiFi using credentials configured via menuconfig.
  display.wifi.begin();
  if (!display.wifi.waitForConnect()) {
    ESP_LOGE(TAG, "WiFi connection failed");

    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor((display.width() / 2) - 200, display.height() / 2);
    display.print("Unable to connect to WiFi!");
    display.setCursor((display.width() / 2) - 200, (display.height() / 2) + 30);
    display.print("Please check SSID and password.");
    display.display();

    // Go back to sleep for a short while, then retry.
    esp_sleep_enable_timer_wakeup(WIFI_RETRY_INTERVAL_US);
    esp_deep_sleep_start();
    return; // Never reached, app_main() restarts on wake.
  }

  ESP_LOGI(TAG, "WiFi connected, fetching a quote...");
  while (!network.getData(quote, sizeof(quote), author, sizeof(author))) {
    ESP_LOGW(TAG, "Fetch failed, retrying...");
    vTaskDelay(pdMS_TO_TICKS(FETCH_RETRY_DELAY_MS));
  }

  display.clearDisplay();

  // Draw the quote inside a text box using a monospace font.
  display.drawTextBox(BOX_LEFT_MARGIN_PX,
                      display.height() / 2 - BOX_TOP_OFFSET_PX,
                      display.width() - BOX_RIGHT_MARGIN_PX,
                      display.height() / 2 + BOX_BOTTOM_OFFSET_PX, quote,
                      BOX_TEXT_SIZE_MULTIPLIER, &FreeMonoBold12pt7b,
                      BOX_LINE_SPACING_PX, false, BOX_WRAP_FONT_SIZE);

  // Print the author in the bottom-right corner.
  uint16_t w, h;
  int16_t x, y;
  display.getTextBounds(author, 0, 0, &x, &y, &w, &h);
  display.setCursor(display.width() - w - AUTHOR_RIGHT_MARGIN_PX,
                    display.height() - AUTHOR_BOTTOM_MARGIN_PX);
  display.print("-");
  display.print(author);
  display.display();

  ESP_LOGI(TAG, "Quote displayed, entering deep sleep");

  // Go to sleep until it's time to fetch the next quote.
  esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
}
