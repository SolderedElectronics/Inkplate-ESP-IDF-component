/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a pre-converted image on Soldered Inkplate 5.
 *
 * @details     Shows how to display an image that was converted to a C header
 *              file using the Soldered online image converter tool. The image
 *              data is stored in image_ex.h and drawn directly to the display
 *              from ESP32 flash.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      image_ex.h header with a converted image array included in the
 *               project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Convert your image using https://tools.soldered.com/tools/image-converter/
 *    Select "Inkplate 5" as the target board.
 * 2) Save the generated header as image_ex.h in the main/ folder.
 * 3) Build and flash to Inkplate 5.
 *
 * Expected output:
 * - The converted image displayed at position (0, 0) on the e-paper screen.
 *
 * Notes:
 * - image_w and image_h are defined in the generated image_ex.h.
 * - For grayscale mode, change BLACK_AND_WHITE to GRAYSCALE and remove
 *   the BLACK color argument from image.draw().
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
#include "image_ex.h"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.image.draw(image, 0, 0, image_w, image_h, BLACK);
  display.display();
}
