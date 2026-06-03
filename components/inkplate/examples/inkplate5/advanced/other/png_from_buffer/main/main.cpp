/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a PNG image loaded into a RAM buffer on Soldered Inkplate 5.
 *
 * @details     Demonstrates how to read a PNG file from an SD card into a RAM
 *              buffer and then display it using the image draw function. The same
 *              technique applies to PNG data received from any source — a network
 *              socket, a serial transfer, a flash partition, etc.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable, microSD card
 * - Extra:      SD card containing a file named "image.png"
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Copy a PNG file named "image.png" to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 5.
 * 4) The PNG is read into RAM and rendered on the e-paper display.
 *
 * Expected output:
 * - The PNG image is shown on the Inkplate display.
 *
 * Notes:
 * - The entire PNG file is loaded into heap memory before decoding.
 *   Make sure the file fits in available RAM (SPIRAM is available).
 * - PNG resolution should not exceed 1280x720 pixels.
 * - Dithering is enabled by default; pass false as the fifth argument to disable.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  display.clearDisplay();

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("SD card error!");
    display.display();
    return;
  }

  FILE *f = fopen("/sdcard/image.png", "rb");
  if (!f) {
    ESP_LOGE(TAG, "Cannot open image.png");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Cannot open image.png");
    display.display();
    display.sdCardSleep();
    return;
  }

  fseek(f, 0, SEEK_END);
  long fileSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (fileSize <= 0) {
    ESP_LOGE(TAG, "Invalid file size: %ld", fileSize);
    fclose(f);
    display.sdCardSleep();
    return;
  }

  ESP_LOGI(TAG, "PNG file size: %ld bytes", fileSize);

  uint8_t *buf = (uint8_t *)malloc((size_t)fileSize);
  if (!buf) {
    ESP_LOGE(TAG, "Not enough RAM for image buffer!");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Not enough RAM!");
    display.display();
    fclose(f);
    display.sdCardSleep();
    return;
  }

  size_t bytesRead = fread(buf, 1, (size_t)fileSize, f);
  fclose(f);
  display.sdCardSleep();

  if (bytesRead != (size_t)fileSize) {
    ESP_LOGE(TAG, "Read mismatch: expected %ld, got %zu", fileSize, bytesRead);
    free(buf);
    return;
  }

  ESP_LOGI(TAG, "File read OK, decoding PNG...");

  if (!display.image.draw(buf, (uint32_t)fileSize, 0, 0, true, false)) {
    ESP_LOGE(TAG, "PNG decode error");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("PNG decode error");
  }

  free(buf);
  display.display();

  ESP_LOGI(TAG, "Done");
}
