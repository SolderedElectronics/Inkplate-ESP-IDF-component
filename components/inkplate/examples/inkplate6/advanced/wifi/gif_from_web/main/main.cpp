/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and play a GIF animation from the web (Inkplate 6).
 *
 * @details     Demonstrates how to connect Inkplate 6 to a WiFi network,
 *              download a GIF file from a URL, and play it back on the
 *              e-paper display using partial updates. The whole file is
 *              downloaded into a buffer first, then played from there.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set GIF_URL to a direct link to a GIF file (no HTML redirect pages).
 * 2) Build and flash to Inkplate 6.
 * 3) The board connects to WiFi, downloads the GIF, and loops it forever.
 *
 * Expected output:
 * - The GIF from GIF_URL plays on the display, centered on the screen,
 *   looping until reset/power-cycled.
 *
 * Notes:
 * - Partial update (and therefore GIF playback) only works in
 *   BLACK_AND_WHITE display mode, hence setDisplayMode(BLACK_AND_WHITE)
 *   below.
 * - The whole GIF file is held in memory at once - make sure the file is
 *   small enough to fit.
 * - e-paper partial refresh takes far longer than a typical GIF frame delay,
 *   so actual playback speed is limited by the panel, not by the file.
 * - leaveOn (last argument, defaults to true) keeps the panel powered
 *   between frames instead of power-cycling it on every partialUpdate()
 *   call; pass false to power the panel down after each frame instead.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6
#error \
    "Wrong board selection for this example, please select Inkplate6 in the boards menu."
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

// Direct link to a GIF file (no HTML redirect pages)
#define GIF_URL ""

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.wifi.begin();
  display.wifi.waitForConnect();

  ESP_LOGI(TAG, "WiFi connected, downloading GIF...");

  // Download and play the GIF, looping forever, centered on the screen.
  if (!display.gif.play(GIF_URL, E_INK_WIDTH / 2 - 125,
                        E_INK_HEIGHT / 2 - 125, false, true, 20, true)) {
    display.setTextColor(0);
    display.setTextSize(3);
    display.println("GIF download/open error");
    display.display();
  }
}
