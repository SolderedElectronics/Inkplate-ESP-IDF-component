/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple deep sleep slideshow example for Soldered Inkplate 5.
 *
 * @details     Demonstrates low-power operation on Inkplate 5 using ESP32 deep
 *              sleep. On each wake-up (timer-based), the board redraws the
 *              screen with the next image in a small slideshow, performs a full
 *              display refresh, and then returns to deep sleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable (or battery for low-power testing)
 * - Extra:      Converted image header files (picture1.h, picture2.h, picture3.h)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Convert 3 grayscale images (1280x720) using the Soldered Image Converter
 *    and replace picture1.h, picture2.h, and picture3.h with the output.
 * 2) Build and flash to Inkplate 5.
 * 3) The board shows an image, goes to deep sleep, and wakes every 20 seconds.
 * 4) After each wake-up, the next image is shown (loops through 3 images).
 *
 * Expected output:
 * - Inkplate display shows a new image every 20 seconds.
 * - The slideshow loops through all provided images.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - RAM contents are lost during deep sleep; standard partial updates cannot
 *   be used across sleep cycles.
 * - This example uses grayscale mode, which requires full refresh updates.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE5
#error \
    "Wrong board selection for this example, please select Inkplate5 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_attr.h"
#include "esp_sleep.h"

#include "picture1.h"
#include "picture2.h"
#include "picture3.h"

#define TIME_TO_SLEEP_US (20ULL * 1000000ULL)

static const uint8_t * const pictures[] = {pic1, pic2, pic3};

RTC_DATA_ATTR static int slide = 0;

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  display.clearDisplay();
  display.image.draw((const uint8_t *)pictures[slide], 0, 0, 1280, 720, 0);
  display.display();

  slide++;
  if (slide > 2)
    slide = 0;

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}
