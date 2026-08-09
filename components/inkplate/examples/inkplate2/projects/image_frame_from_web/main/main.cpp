/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Random web image frame with periodic deep sleep refresh
 *              (Inkplate 2).
 *
 * @details     Demonstrates a simple "image frame" project on Inkplate 2. On
 *              every boot, the board connects to WiFi and downloads a
 *              randomly generated image from LoremFlickr, sized to match the
 *              Inkplate 2 resolution (212x104), then renders it on the
 *              e-paper display with a full refresh. The device then enters
 *              deep sleep for SECS_BETWEEN_IMAGES seconds. Because deep
 *              sleep resets the ESP32, the program always restarts from
 *              app_main() on every wake cycle and fetches a new random
 *              image, so there is no explicit refresh loop.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 * - Refresh period: set SECS_BETWEEN_IMAGES below (seconds)
 *
 * How to use:
 * 1) Configure WiFi credentials via menuconfig.
 * 2) Build and flash to Inkplate 2.
 * 3) On boot, the device downloads a random 212x104 image and displays it.
 * 4) The device enters deep sleep and repeats after SECS_BETWEEN_IMAGES
 *    seconds.
 *
 * Expected output:
 * - Display: a randomly selected image rendered full-screen (212x104).
 * - Log: the requested image URL and the image.draw() result.
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode.
 * - display.image.draw() downloads over HTTP/HTTPS and follows redirects
 *   internally, so the manual HTTPClient + WiFiClientSecure redirect
 *   resolution used by the original Arduino sketch is not needed here.
 * - Deep sleep restarts the ESP32; all logic lives in app_main() and there
 *   is no loop().
 * - Web/API behaviour can change: if the image provider changes its
 *   response format, requests may start failing.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_sleep.h"

static const char *TAG = "IMAGE_FRAME";

// ---------------- CHANGE HERE ---------------------:

// Random image endpoint, constrained to Inkplate 2 resolution (212x104).
// TODO: replace with your own image URL/service if desired.
#define IMAGE_URL "http://loremflickr.com/212/104"

// Delay between two images, in seconds.
#define SECS_BETWEEN_IMAGES 30

// ---------------------------------------------------

extern "C" void app_main(void) {
  Inkplate display;

  // Join WiFi using credentials configured via menuconfig.
  display.wifi.begin();
  display.wifi.waitForConnect();

  // Download and draw the image directly from the web. image.draw() detects
  // the image format (BMP/JPEG/PNG) automatically and follows HTTP
  // redirects, which is how the random image endpoint resolves to an actual
  // image file. Returns true on success and false on failure.
  ESP_LOGI(TAG, "Requesting image: %s", IMAGE_URL);
  bool ok = display.image.draw(IMAGE_URL, 0, 0, true, false);
  ESP_LOGI(TAG, "image.draw() result: %d", ok);

  display.display();

  ESP_LOGI(TAG, "Going to sleep for %d seconds", SECS_BETWEEN_IMAGES);

  // Activate wakeup timer.
  esp_sleep_enable_timer_wakeup((uint64_t)SECS_BETWEEN_IMAGES * 1000000ULL);

  // Start deep sleep (this function does not return). The ESP32 resets and
  // re-runs app_main() on wake, so no explicit loop is needed here.
  esp_deep_sleep_start();
}
