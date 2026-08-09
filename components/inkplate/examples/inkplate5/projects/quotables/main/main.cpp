/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetch a random quote from a public API and display it on
 *              Soldered Inkplate 5, then deep sleep and refresh periodically.
 *
 * @details     This example demonstrates calling a simple public REST API
 *              that returns JSON (no authentication required), extracting
 *              the quote text and author using cJSON via a QuotablesNetwork
 *              helper, and displaying the result on Inkplate 5.
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
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable (battery optional)
 * - Extra:      Stable WiFi connection + Internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Build and flash to Inkplate 5.
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

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE5
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate5 in the boards menu."
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

// Buffers used to hold the fetched quote and author name.
static char quote[128];
static char author[64];

extern "C" void app_main(void) {
  Inkplate display;
  NetworkFunctions network;

  display.setTextColor(BLACK);
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
  display.drawTextBox(48, display.height() / 2 - 36, display.width() - 48,
                      display.height() / 2 + 200, quote, 1,
                      &FreeMonoBold24pt7b, 36, false, 38);

  // Print the author in the bottom-right corner.
  uint16_t w, h;
  int16_t x, y;
  display.getTextBounds(author, 0, 0, &x, &y, &w, &h);
  display.setCursor(display.width() - w - 50, display.height() - 30);
  display.print("-");
  display.print(author);
  display.display();

  ESP_LOGI(TAG, "Quote displayed, entering deep sleep");

  // Go to sleep until it's time to fetch the next quote.
  esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
}
