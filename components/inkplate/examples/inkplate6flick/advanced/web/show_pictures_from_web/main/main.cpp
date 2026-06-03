/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and display images from the web (Inkplate 6 Flick).
 *
 * @details     Demonstrates how to connect Inkplate 6 Flick to a WiFi network,
 *              download an image from a web URL, and render it on the e-paper
 *              display using the Inkplate image drawing functions.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set the image URL to a compatible image file.
 * 2) Build and flash to Inkplate 6 Flick.
 * 3) The board connects to WiFi, downloads the image, and displays it.
 *
 * Expected output:
 * - Image downloaded from the web is displayed on the Inkplate screen.
 *
 * Notes:
 * - Supported formats include BMP and JPEG.
 * - Images must fit the display; large images may not render properly.
 * - Ensure the URL is directly pointing to the image file.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BMP_IMAGE_URL  "https://varipass.org/neowise_mono.bmp"
#define JPEG_IMAGE_URL "https://varipass.org/destination.jpg"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.wifi.begin();
  display.wifi.waitForConnect();

  // Draw BMP image from URL
  if (!display.image.draw(BMP_IMAGE_URL, 0, 0, false, true)) {
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("BMP image open error");
  }
  display.display();

  vTaskDelay(pdMS_TO_TICKS(5000));
  display.clearDisplay();

  // Draw JPEG image from URL
  if (!display.image.draw(JPEG_IMAGE_URL, 0, 100, true, false)) {
    display.setTextSize(2);
    display.setCursor(0, 100);
    display.print("JPEG image open error");
  }
  display.display();
}
