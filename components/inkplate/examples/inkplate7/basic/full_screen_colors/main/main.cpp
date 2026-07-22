/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Full-screen color bars example for Soldered Inkplate 7.
 *
 * @details     Fills the entire Inkplate 7 screen with six vertical color
 *              bars — one for each supported color. Useful as a quick
 *              visual test to verify all six e-paper colors render correctly.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Build and flash to Inkplate 7.
 * 2) The display shows six vertical color bars.
 *
 * Expected output:
 * - Six full-height vertical bars in: black, white, yellow, red, blue, green.
 *
 * Notes:
 * - display.display() must be called to update the physical e-paper panel.
 * - Inkplate 7 supports 6 colors: black, white, yellow, red, blue, green.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE7
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate7 in the boards menu."
#endif

#include "Inkplate.h"

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.fillRect(0 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_BLACK);
  display.fillRect(1 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_WHITE);
  display.fillRect(2 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_YELLOW);
  display.fillRect(3 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_RED);
  display.fillRect(4 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_BLUE);
  display.fillRect(5 * 800 / 6, 0, 800 / 6 + 2, 480, INKPLATE_GREEN);

  display.display();
}
