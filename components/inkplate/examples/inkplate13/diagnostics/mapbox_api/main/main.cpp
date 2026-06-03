/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Mapbox API map display example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Connects to WiFi, fetches a map tile from the Mapbox Static
 *              Images API, displays it on the e-paper screen, then enters deep
 *              sleep for 5 minutes before waking and repeating.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      Stable WiFi connection, Mapbox API key
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Get a free API key from https://www.mapbox.com/
 * 2) Fill in API_KEY and adjust the bounding box coordinates (LAT1/LON1/LAT2/LON2).
 *    Use http://bboxfinder.com/ to find coordinates for your area of interest.
 * 3) Build and flash to Inkplate 13SPECTRA.
 * 4) The board connects to WiFi, downloads and displays the map, then sleeps.
 *
 * Expected output:
 * - A Mapbox map tile displayed on the Inkplate 13SPECTRA e-paper screen.
 * - Board enters deep sleep for 5 minutes then wakes and refreshes.
 *
 * Notes:
 * - Deep sleep resets normal RAM; the whole program runs in app_main each wake.
 * - Adjust TIME_TO_SLEEP_US to change the refresh interval.
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

#define API_KEY "" // Your Mapbox API key from https://www.mapbox.com/
#define LAT1    18.679247
#define LON1    45.543870
#define LAT2    18.715210
#define LON2    45.562021

#define TIME_TO_SLEEP_US (5ULL * 60ULL * 1000000ULL)

static const char *TAG = "mapbox_api";

extern "C" void app_main(void) {
  Inkplate display;

  display.wifi.begin();
  display.wifi.waitForConnect();

  char url[256];
  snprintf(url, sizeof(url),
           "https://api.mapbox.com/styles/v1/mapbox/navigation-day-v1/static"
           "/[%lf,%lf,%lf,%lf]/1600x1200?access_token=%s",
           LAT1, LON1, LAT2, LON2, API_KEY);

  ESP_LOGI(TAG, "Fetching: %s", url);

  display.image.draw(url, 0, 0, false, false);
  display.display();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}
