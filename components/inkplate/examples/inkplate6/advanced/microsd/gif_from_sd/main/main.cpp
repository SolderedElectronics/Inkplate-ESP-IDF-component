/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Play a GIF animation from the SD card on Soldered Inkplate 6.
 *
 * @details     Demonstrates loading a GIF file from a FAT-formatted SD card
 *              and playing it back on the Inkplate 6 e-paper display using
 *              partial updates. Each pixel is converted to black/white with
 *              a fixed threshold (no dithering) since dithering noise would
 *              flicker differently frame to frame.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6, USB cable, microSD card
 * - Extra:      SD card containing a GIF file named "cat_gif.gif"
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Copy the cat_gif.gif file bundled with this example to the root of
 *    a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 6.
 * 4) The GIF plays back on the e-paper screen.
 *
 * Expected output:
 * - "cat_gif.gif" loops forever on the display, centered on the screen.
 *
 * Notes:
 * - Partial update (and therefore GIF playback) only works in
 *   BLACK_AND_WHITE display mode, hence setDisplayMode(BLACK_AND_WHITE)
 *   below.
 * - e-paper partial refresh takes far longer than a typical GIF frame delay
 *   (tens to hundreds of ms per refresh vs ~100ms/frame in the file), so
 *   actual playback speed is limited by the panel, not by the GIF itself.
 * - The driver forces a full refresh every N partial updates
 *   (fullRefreshEveryFrames argument, defaults to 20 here) to clear
 *   partial-update ghosting; pass 0 to disable forced full refreshes
 *   entirely.
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

// GIF file on the SD card root
#define GIF_PATH "cat_gif.gif"

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  // Init SD card. Display if SD card is init properly or not.
  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setTextColor(0);
    display.setTextSize(3);
    display.print("SD Card error!");
    display.display();
    return;
  }

  // Play "cat_gif.gif" from the SD card root, centered on the screen.
  // loop = true: keep replaying the file forever.
  if (!display.gif.play(GIF_PATH, E_INK_WIDTH / 2 - 125,
                        E_INK_HEIGHT / 2 - 125, false, true, 20, true)) {
    display.setTextColor(0);
    display.setTextSize(3);
    display.println("GIF open error");
    display.display();
  }

  display.sdCardSleep();
}
