/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Basic "Hello World" example for Soldered Inkplate 7.
 *
 * @details     Demonstrates the most basic usage of the Inkplate 7 by
 *              initializing the display and printing "Hello World!" on the
 *              e-paper screen. The example uses built-in text rendering
 *              functions fully compatible with the Adafruit GFX library.
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
 * 2) After initialization, "Hello World!" appears on the display.
 *
 * Expected output:
 * - The text "Hello World!" displayed on the Inkplate screen.
 *
 * Notes:
 * - display.clearDisplay() clears only the internal framebuffer.
 * - display.display() must be called to update the physical e-paper panel.
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

  display.clearDisplay(); // Clear the frame buffer (does NOT clear the physical
                          // screen)
  display.setCursor(10, 10);     // Set the text position to (10, 10) pixels
  display.setTextSize(6);        // Set text size to 6 (default is 1)
  display.setTextColor(INKPLATE_BLACK); // Set text color to black
  display.print("Hello World!"); // Print "Hello World!" at the set position
  display.display();             // Refresh the e-paper display to show changes
}
