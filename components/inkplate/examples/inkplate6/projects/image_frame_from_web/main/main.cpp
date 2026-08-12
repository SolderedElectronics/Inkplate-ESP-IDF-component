/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Digital picture frame for Soldered Inkplate 6.
 *
 * @details     Connects Inkplate 6 to WiFi, downloads a JPEG image from a
 *              URL, and renders it full-screen in 3-bit grayscale. After
 *              displaying the image, the ESP32 sets a wake-up timer and
 *              enters deep sleep. When the timer expires, the ESP32 restarts
 *              from app_main(), causing the image to be downloaded and
 *              displayed again — creating a periodically refreshing image
 *              frame.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6, USB cable (or battery for low-power testing)
 * - Extra:      Stable WiFi (2.4 GHz) Internet connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID and password in menuconfig.
 * 2) (Optional) Change IMAGE_URL below to point to your own image.
 * 3) Build and flash to Inkplate 6.
 * 4) The board connects to WiFi, downloads the image, and displays it.
 * 5) The device enters deep sleep and wakes periodically to refresh the
 *    image.
 *
 * Expected output:
 * - E-paper: A JPEG image rendered full-screen on the display in 3-bit
 *   grayscale.
 * - Serial: WiFi join status and image draw result.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (GRAYSCALE). Partial update is not
 *   available in grayscale mode; this example uses a full refresh via
 *   display.display().
 * - Deep sleep restarts the ESP32; app_main() runs again on every wake-up.
 * - HTTPS certificate validation is disabled for this example via
 *   CONFIG_ESP_TLS_INSECURE / CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY
 *   (sdkconfig.defaults). This is fine for demo/testing only; for
 *   production use, validate TLS properly (e.g. wifi.setCertificate()
 *   with a matching CA certificate).
 * - display.image.draw() downloads the file with esp_http_client, which
 *   follows HTTP redirects automatically, so no manual redirect
 *   resolution is needed even though the default URL below redirects to
 *   a random image on another host.
 * - Web images and decoding can be RAM-intensive. Large JPEGs or complex
 *   images may fail to decode depending on available memory.
 * - Network endpoints can change behavior (redirects, user-agent
 *   filtering, rate limits). If downloads fail, check the log and try a
 *   different image source.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "IMAGE_FRAME_WEB";

// Public test endpoint that redirects to a random 800x600 photo. Replace
// with a URL pointing to your own image if desired.
#define IMAGE_URL "http://loremflickr.com/800/600"

// How long to sleep between refreshes.
#define REFRESH_INTERVAL_MIN 15
#define REFRESH_INTERVAL_US                                                  \
  ((uint64_t)REFRESH_INTERVAL_MIN * 60ULL * 1000000ULL)

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  display.wifi.begin();
  display.wifi.waitForConnect();

  ESP_LOGI(TAG, "WiFi connected, downloading image...");

  if (!display.image.draw(IMAGE_URL, 0, 0, true, false)) {
    ESP_LOGE(TAG, "Image draw failed");
    display.setTextSize(2);
    // GRAYSCALE mode uses raw 0-7 gray levels (0=black, 7=white), not the
    // BLACK/WHITE macros (1/0), which are only correct in BLACK_AND_WHITE
    // mode - using WHITE(0) here would paint a black background.
    display.setTextColor(0, 7);
    display.setCursor(20, 280);
    display.print("Image download error");
  }

  display.display();

  ESP_LOGI(TAG, "Going to sleep for %d minutes", REFRESH_INTERVAL_MIN);

  // Activate wake-up timer and enter deep sleep. This function does not
  // return; app_main() runs again from the top when the ESP32 wakes up.
  esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
}
