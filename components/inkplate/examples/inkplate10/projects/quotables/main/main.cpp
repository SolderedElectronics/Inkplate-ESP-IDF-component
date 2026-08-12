/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetch a random quote from a public API and display it on
 *              Soldered Inkplate 10, then deep sleep and refresh periodically.
 *
 * @details     This example demonstrates calling a simple public REST API
 *              that returns JSON (no authentication required), extracting
 *              the quote text and author using cJSON via a QuotablesNetwork
 *              helper, and displaying the result on Inkplate 10.
 *
 *              The quote is rendered inside a text box using drawTextBox()
 *              with a large monospace font, and the author is printed in the
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
 *              Layout note: Inkplate 10's panel (1200x825) is both larger and
 *              has a different aspect ratio than Inkplate 6's (800x600). The
 *              original Arduino sketch reused Inkplate 6's fixed pixel
 *              margins verbatim, which would leave disproportionate empty
 *              space around the quote box on the bigger panel. Here the
 *              margins/offsets that scale with screen size (side margins,
 *              and how far the box extends above/below vertical center) are
 *              scaled by the width/height ratio versus Inkplate 6
 *              (1200/800 = 1.5x, 825/600 = 1.375x). The font-metric-derived
 *              constants (line spacing, wrap-width estimate) are left
 *              unchanged, since the fonts themselves are the same bitmap
 *              fonts at the same pixel size regardless of panel resolution.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable (battery optional)
 * - Extra:      Stable WiFi connection + Internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Build and flash to Inkplate 10.
 * 3) On every boot/wake, the device fetches a quote and updates the display,
 *    then deep sleeps for the configured interval.
 *
 * Expected output:
 * - Display: the fetched quote centered in a text box using a large
 *   monospace font; the author printed at the bottom-right, prefixed with
 *   "-".
 * - Serial Monitor: connection status and fetch retry logs.
 * - On WiFi failure: an "Unable to connect" message, then a short sleep and
 *   automatic retry.
 *
 * Notes:
 * - Display mode is 1-bit (BW). This example uses a full refresh (display()).
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

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "QuotablesNetwork.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fonts/FreeMonoBold24pt7b.h"

static const char *TAG = "QUOTABLES";

// Delay between quote refreshes, in microseconds (5 minutes).
#define REFRESH_INTERVAL_US (300ULL * 1000000ULL)
// Delay before retrying after a WiFi connection failure, in microseconds.
#define WIFI_RETRY_INTERVAL_US (5ULL * 1000000ULL)
// Delay between failed quote-fetch attempts, in milliseconds.
#define FETCH_RETRY_DELAY_MS 1000

// Quote text box layout, scaled from Inkplate 6's 800x600 tuning to
// Inkplate 10's 1200x825 panel (see @details above). Side margins scale with
// the 1.5x width ratio; how far the box reaches above/below vertical center
// scales with the 1.375x height ratio.
#define BOX_SIDE_MARGIN_PX 72   // left/right inset from the panel edges
#define BOX_TOP_OFFSET_PX 50    // box top = height/2 - this
#define BOX_BOTTOM_OFFSET_PX 275 // box bottom = height/2 + this
// Font-metric-derived constants: tied to the (unchanged) bitmap font, not to
// panel resolution, so they are kept the same as the Inkplate 6 tuning.
#define BOX_LINE_SPACING_PX 36  // vertical spacing between wrapped lines
#define BOX_WRAP_FONT_SIZE 38   // estimated glyph width used for word-wrap

// Author signature layout, scaled the same way as the quote box.
#define AUTHOR_RIGHT_MARGIN_PX 75 // right inset for the right-aligned author
#define AUTHOR_BOTTOM_MARGIN_PX 40 // bottom inset for the author baseline

// Buffers used to hold the fetched quote and author name.
static char quote[128];
static char author[64];

extern "C" void app_main(void) {
  Inkplate display;
  NetworkFunctions network;

  // Display mode defaults to GRAYSCALE (never switched here), which uses raw
  // 0-7 gray levels (0=black, 7=white), not the BLACK macro (1), which is
  // only correct in BLACK_AND_WHITE mode.
  display.setTextColor(0);
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

  // Draw the quote inside a text box using a large monospace font.
  display.drawTextBox(BOX_SIDE_MARGIN_PX,
                      display.height() / 2 - BOX_TOP_OFFSET_PX,
                      display.width() - BOX_SIDE_MARGIN_PX,
                      display.height() / 2 + BOX_BOTTOM_OFFSET_PX, quote, 1,
                      &FreeMonoBold24pt7b, BOX_LINE_SPACING_PX, false,
                      BOX_WRAP_FONT_SIZE);

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
