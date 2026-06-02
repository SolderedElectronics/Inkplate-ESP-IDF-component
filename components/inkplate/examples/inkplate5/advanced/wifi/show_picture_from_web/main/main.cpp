/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and display images from the web (Inkplate 5).
 *
 * @details     Demonstrates how to connect Inkplate 5 to a WiFi network,
 *              download an image from a web URL, and render it on the e-paper
 *              display using the Inkplate image drawing functions.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set the image URL to a compatible image file.
 * 2) Build and flash to Inkplate 5.
 * 3) The board connects to WiFi, downloads the image, and displays it.
 *
 * Expected output:
 * - Image downloaded from the web is displayed on the Inkplate screen.
 *
 * Notes:
 * - Supported formats include BMP, JPEG, and PNG.
 * - Images must fit the display; large images may not render properly.
 * - Ensure the URL is directly pointing to the image file.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define IMAGE_PATH "https://varipass.org/neowise_mono.bmp"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.wifi.begin();
  display.wifi.waitForConnect();

  display.image.draw(IMAGE_PATH, 0, 0, true, false);
  display.display();
}
