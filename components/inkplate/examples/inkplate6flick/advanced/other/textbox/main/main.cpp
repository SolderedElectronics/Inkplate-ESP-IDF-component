/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Text box with word wrap example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates the drawTextBox() function which renders long text
 *              within a defined rectangular region with automatic word
 *              wrapping. Two text boxes are shown side by side: one using the
 *              default built-in font, and one using a custom Roboto Light 36 pt
 *              font with configurable line spacing.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      Roboto_Light_36.h custom font header included in the project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Two text boxes with the same sample text appear side by side.
 *
 * Expected output:
 * - Left box: sample text in the default font with word wrap.
 * - Right box: same text in Roboto Light 36 pt with 27 px line spacing.
 *
 * Notes:
 * - Text that exceeds the box bounds is truncated with "...".
 * - Some custom fonts are drawn bottom-to-top and require a vertical offset.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "Roboto_Light_36.h"

const char *text =
    "This is an example of a text written in a textbox. When a word doesn't "
    "fit into the current row, it goes to the next one."
    " If the text reaches the lower bound, it ends with three dots (...) to "
    "mark that the text isnt displayed fully";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  // Left box: default font, no extra options
  display.drawTextBox(100, 100, 450, 600, text);

  // Right box: Roboto Light 36 pt, 27 px line spacing
  int offset = 32;
  display.drawTextBox(550, 100 + offset, 950, 600, text, 1, &Roboto_Light_36,
                      27, false, 36);

  display.display();
}
