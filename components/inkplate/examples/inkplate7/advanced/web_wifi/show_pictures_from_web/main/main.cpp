/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and display images from the web (Inkplate 7).
 *
 * @details     Demonstrates how to connect Inkplate 7 to a WiFi
 *              network, download BMP and JPG images from URLs, and render them
 *              on the e-paper display using the Inkplate image drawing
 *              functions. Supports BMP (1/4/8/24-bit) and JPG formats.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Enter your WiFi credentials via menuconfig.
 * 2) Build and flash to Inkplate 7.
 * 3) Board connects to WiFi, downloads and displays three images in sequence.
 *
 * Expected output:
 * - Three images displayed one after another on the Inkplate screen.
 *
 * Notes:
 * - BMP images must be Windows BMP format, 1/4/8/24-bit, no compression.
 * - Images larger than 800x480 will not fit on the display.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE7
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate7 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHOW_PICTURES";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.display();

  display.wifi.begin();
  display.wifi.waitForConnect();

  // Monochromatic BMP (1-bit depth) — loads fastest
  ESP_LOGI(TAG, "Downloading neowise_mono.bmp...");
  if (!display.image.draw("https://varipass.org/neowise_mono.bmp", 0, 0, true,
                          false)) {
    display.setCursor(0, 0);
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
    display.print("Image open error");
    display.display();
    ESP_LOGE(TAG, "Failed to draw neowise_mono.bmp");
  } else {
    display.display();
  }

  vTaskDelay(pdMS_TO_TICKS(3000));

  // Color BMP
  ESP_LOGI(TAG, "Downloading neowise.bmp...");
  if (!display.image.draw("https://varipass.org/neowise.bmp", 0, 0, true,
                          false)) {
    display.setCursor(0, 0);
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
    display.print("Image open error");
    display.display();
    ESP_LOGE(TAG, "Failed to draw neowise.bmp");
  } else {
    display.display();
  }

  vTaskDelay(pdMS_TO_TICKS(3000));
  display.clearDisplay();

  // JPG image
  ESP_LOGI(TAG, "Downloading destination.jpg...");
  if (!display.image.draw("https://varipass.org/destination.jpg", 0, 100, true,
                          false)) {
    display.setCursor(0, 100);
    display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
    display.print("Image open error");
    display.display();
    ESP_LOGE(TAG, "Failed to draw destination.jpg");
  } else {
    display.display();
  }
}
