/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and display BMP/JPG images from the web (Inkplate
 *              4TEMPERA).
 *
 * @details     Demonstrates how to connect Inkplate 4TEMPERA to a WiFi network,
 *              download a BMP image from a web URL, and render it on the
 *              e-paper display using the Inkplate image drawing functions.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set your WiFi SSID and password in menuconfig.
 * 2) Build and flash to Inkplate 4TEMPERA.
 * 3) The board connects to WiFi, downloads the image, and displays it.
 *
 * Expected output:
 * - BMP image downloaded from the web is displayed on the Inkplate screen.
 *
 * Notes:
 * - Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth.
 * - Images must fit the display; large images may not render correctly.
 * - Ensure the URL points directly to the image file.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHOW_PIC_WEB";

// Monochromatic BMP — loads quickest. Photo by Roberto Fernandez.
#define IMAGE_URL "https://varipass.org/neowise_mono.bmp"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  display.wifi.begin();
  display.wifi.waitForConnect();

  ESP_LOGI(TAG, "WiFi connected, downloading image...");

  if (!display.image.draw(IMAGE_URL, 0, 0, false, true)) {
    ESP_LOGE(TAG, "Image draw failed");
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(20, 280);
    display.print("Image open error");
  }

  display.display();

  ESP_LOGI(TAG, "Done");
}
