/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Text box rendering example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates how to use the drawTextBox function with and
 *              without optional parameters. One text box uses the default
 *              built-in font; the other uses the Roboto Light 36pt custom font
 *              with explicit vertical spacing and no border.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Build and flash to Inkplate 13SPECTRA.
 * 2) Two text boxes are shown on the display simultaneously.
 *
 * Expected output:
 * - Left text box uses the default font.
 * - Right text box uses Roboto Light 36pt with custom spacing.
 *
 * Notes:
 * - drawTextBox truncates text with "..." when it reaches the lower bound.
 * - Some custom fonts are drawn bottom-to-top; use the offset parameter to
 *   compensate (32 px for Roboto_Light_36).
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
#include "Roboto_Light_36.h"

static const char *text =
    "This is an example of a text written in a textbox. When a word doesn't "
    "fit into the current row, it goes to the next one. If the text reaches "
    "the lower bound, it ends with three dots (...) to mark that the text "
    "isn't displayed fully";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.display();
  display.setTextColor(INKPLATE_BLACK);

  display.drawTextBox(100, 100, 500, 500, text, 2);

  int offset = 32;
  display.drawTextBox(700, 100 + offset, 900, 300, text, 1, &Roboto_Light_36,
                      27, false, 36);

  display.display();
}
