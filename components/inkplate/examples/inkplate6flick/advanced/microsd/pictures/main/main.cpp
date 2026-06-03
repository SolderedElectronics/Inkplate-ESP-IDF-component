/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display images from SD card on Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to load BMP and JPEG image files from an SD
 *              card and display them sequentially on the Inkplate 6 Flick
 *              e-paper display in grayscale mode.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable, microSD card
 * - Extra:      image1.bmp, image2.bmp, pyramid.jpg on SD card root
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Format a microSD card as FAT/FAT32.
 * 2) Copy image1.bmp, image2.bmp, and pyramid.jpg to the SD card root.
 * 3) Insert the SD card into Inkplate 6 Flick.
 * 4) Build and flash.
 * 5) Images are displayed sequentially with 5-second delays.
 *
 * Expected output:
 * - "SD Card OK!" message, then image1.bmp, image2.bmp, pyramid.jpg in sequence.
 * - Error message shown on display if a file cannot be opened.
 *
 * Notes:
 * - Supported formats: BMP (1/4/8/24-bit uncompressed), JPEG, PNG.
 * - BMP files must be uncompressed.
 * - SD card is powered off after display to reduce power consumption.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  display.clearDisplay();
  display.setTextColor(0);
  display.setTextSize(3);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.print("SD Card error!");
    display.display();
    return;
  }

  display.print("SD Card OK! Loading image...");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(2000));

  display.clearDisplay();
  if (!display.image.draw("image1.bmp", 0, 0, true, false)) {
    display.clearDisplay();
    display.print("Image open error: image1.bmp");
  }
  display.display();
  vTaskDelay(pdMS_TO_TICKS(5000));

  display.clearDisplay();
  if (!display.image.draw("image2.bmp", 0, 0, true, false)) {
    display.clearDisplay();
    display.print("Image open error: image2.bmp");
  }
  display.display();
  vTaskDelay(pdMS_TO_TICKS(5000));

  display.clearDisplay();
  if (!display.image.draw("pyramid.jpg", 100, 0, true, false)) {
    display.clearDisplay();
    display.print("Image open error: pyramid.jpg");
  }
  display.display();

  display.sdCardSleep();
}
