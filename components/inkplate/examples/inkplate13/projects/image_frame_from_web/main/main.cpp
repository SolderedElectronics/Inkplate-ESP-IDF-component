/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Digital picture frame for Soldered Inkplate 13SPECTRA.
 *
 * @details     Connects Inkplate 13SPECTRA to WiFi, downloads a JPEG image
 *              from a URL, and renders it full-screen, dithered down to the
 *              panel's 6-color palette. After displaying the image, the
 *              ESP32 sets a wake-up timer and enters deep sleep. When the
 *              timer expires, the ESP32 restarts from app_main(), causing
 *              the image to be downloaded and displayed again - creating a
 *              periodically refreshing image frame.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable (or battery for low-power
 *               testing)
 * - Extra:      Stable WiFi (2.4 GHz) Internet connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID and password in menuconfig.
 * 2) (Optional) Change IMAGE_URL below to point to your own image.
 * 3) Build and flash to Inkplate 13SPECTRA.
 * 4) The board connects to WiFi, downloads the image, and displays it.
 * 5) The device enters deep sleep and wakes periodically to refresh the
 *    image.
 *
 * Expected output:
 * - E-paper: A JPEG image rendered full-screen on the display, dithered to
 *   the panel's 6-color palette.
 * - Serial: WiFi join status and image draw result.
 *
 * Notes:
 * - Color image path: Inkplate 13SPECTRA uses ImageColor (not the plain
 *   grayscale Image class), so display.image.draw() dithers/quantizes the
 *   downloaded JPEG down to the panel's 6 colors: black, white, yellow, red,
 *   blue, green. There is no INKPLATE_ORANGE on this board, unlike Inkplate
 *   6Color's 7-color palette. ImageColor already knows this board's palette
 *   internally (see components/inkplate/src/graphics/ImageColor.cpp, which
 *   branches on CONFIG_INKPLATE_BOARD_INKPLATE13), so no palette handling is
 *   needed here.
 * - `display` is intentionally a LOCAL variable in app_main(), not a
 *   file-scope global. A file-scope `Inkplate display;` would race the
 *   library's own global I2C/PCAL peripheral objects (in BoardCommon.cpp) -
 *   C++ leaves cross-translation-unit static init order unspecified, so the
 *   Inkplate constructor could run before the I2C bus/expander objects it
 *   depends on, leaving peripherals uninitialized.
 * - Redirect handling: the original Inkplate13SPECTRA_Image_Frame_From_Web
 *   sketch manually resolves the "Location" header via a raw HTTPClient
 *   (collectHeaders()/setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS)/
 *   getLocation()) before downloading the image, because the default URL
 *   below (loremflickr.com) responds with an HTTP redirect to a randomly
 *   picked photo on another host. That manual resolution step is dropped
 *   here: display.image.draw()'s underlying downloader
 *   (WiFi::downloadFile()/downloadFileHTTPS(), in
 *   components/inkplate/src/features/WiFi.cpp) already follows up to 5 HTTP
 *   redirects itself by reading the "Location" response header, so passing
 *   IMAGE_URL straight to display.image.draw() is enough - no manual
 *   redirect code is required.
 * - HTTPS certificate validation is disabled for this example via
 *   CONFIG_ESP_TLS_INSECURE / CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY
 *   (sdkconfig.defaults), for parity with other ported examples. The
 *   default IMAGE_URL below is plain HTTP, so this isn't actually exercised;
 *   if you point IMAGE_URL at an HTTPS host, note that this disables server
 *   certificate checking - fine for demo/testing only, not production use.
 * - WiFi connection failure is not specially handled: display.wifi.begin()
 *   / waitForConnect() use their default (10 s) timeout, and if it doesn't
 *   connect in time, the subsequent display.image.draw() call simply fails
 *   (no network), which is caught below and shown as an on-screen error
 *   message. The device still goes to sleep and retries on the next
 *   wake-up cycle either way, matching the original sketch's single
 *   straight-line flow (no WiFi-connected branching).
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

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "IMAGE_FRAME_WEB";

// Public test endpoint that redirects to a random 1600x1200 photo, matching
// the Inkplate 13SPECTRA panel's native resolution. Replace with a URL
// pointing to your own image if desired.
#define IMAGE_URL "http://loremflickr.com/1600/1200"

// How long to sleep between refreshes.
#define REFRESH_INTERVAL_MIN 15
#define REFRESH_INTERVAL_US                                                  \
  ((uint64_t)REFRESH_INTERVAL_MIN * 60ULL * 1000000ULL)

extern "C" void app_main(void) {
  Inkplate display;

  display.wifi.begin();
  display.wifi.waitForConnect();

  ESP_LOGI(TAG, "WiFi connected, downloading image...");

  if (!display.image.draw(IMAGE_URL, 0, 0, true, false)) {
    ESP_LOGE(TAG, "Image draw failed");
    display.setTextSize(2);
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
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
