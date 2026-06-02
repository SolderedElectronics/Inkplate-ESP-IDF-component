/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple deep sleep slideshow example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates low-power operation on Inkplate 4TEMPERA using ESP32
 *              deep sleep. On each wake-up (timer-based), the board redraws the
 *              screen with the next image in a small slideshow, performs a full
 *              display refresh, and then returns to deep sleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Converted image header files (picture1.h, picture2.h, picture3.h)
 *               Convert 600x600 images at: https://tools.soldered.com/tools/image-converter/
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Convert 3 images (600x600 px) using the Soldered Image Converter.
 * 2) Copy the generated picture1.h, picture2.h, picture3.h into this folder.
 * 3) Build and flash to Inkplate 4TEMPERA.
 * 4) The board shows an image, goes to deep sleep, and wakes every 20 seconds.
 * 5) After each wake-up, the next image is shown (loops through 3 images).
 *
 * Expected output:
 * - Inkplate display shows a new image every 20 seconds.
 * - The slideshow loops through all provided images.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - RAM contents are lost during deep sleep; standard partial updates cannot be used.
 * - This example uses 3-bit (grayscale) mode, which requires full refresh updates.
 * - Frontlight and touchscreen are disabled before sleep to save power.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "picture1.h"
#include "picture2.h"
#include "picture3.h"

#define TIME_TO_SLEEP_US (20ULL * 1000000ULL)

static const uint8_t *pictures[] = {picture1, picture2, picture3};

RTC_DATA_ATTR static int slide = 0;

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(GRAYSCALE);
  display.clearDisplay();
  display.image.draw(pictures[slide], 0, 0, 600, 600, 0);
  display.display();

  slide++;
  if (slide > 2)
    slide = 0;

  display.frontlight.setState(false);
  display.touchscreen.shutdown();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}
