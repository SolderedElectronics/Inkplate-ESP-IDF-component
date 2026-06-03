/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a pre-converted image on Soldered Inkplate 13SPECTRA.
 *
 * @details     Shows how to display an image that was converted to a C header
 *              file using the Soldered online image converter tool. The image
 *              data is stored in image.h and drawn directly to the display
 *              from ESP32-S3 flash.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      image.h header with a converted image array included in the project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Convert your image using https://tools.soldered.com/tools/image-converter/
 *    Select "Inkplate 13SPECTRA" as the target board and color mode.
 * 2) Save the generated header as image.h in the main/ folder.
 * 3) Build and flash to Inkplate 13SPECTRA.
 * 4) The image appears on the display after initialization.
 *
 * Expected output:
 * - The converted image displayed at position (0, 0) on the e-paper screen.
 *
 * Notes:
 * - image_w and image_h are automatically defined in the generated image.h.
 * - display.clearDisplay() clears only the internal framebuffer.
 * - display.display() must be called to update the physical e-paper panel.
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
#include "image_ex.h"

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();

  display.image.draw(image, 0, 0, image_w, image_h, 0);

  display.display();
}
