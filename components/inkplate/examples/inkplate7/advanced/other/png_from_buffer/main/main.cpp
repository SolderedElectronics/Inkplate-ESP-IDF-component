/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a PNG from RAM buffer on Soldered Inkplate 7.
 *
 * @details     Reads a PNG file from the microSD card into a heap buffer and
 *              decodes it directly from RAM using image.draw(). The same
 *              technique applies to PNG data from any source — network socket,
 *              serial transfer, flash partition, etc.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable, microSD card
 * - Extra:      SD card containing a file named "image.png"
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Copy a PNG file named "image.png" to a FAT32-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 7.
 * 4) The PNG is read into RAM and rendered on the e-paper display.
 *
 * Expected output:
 * - The PNG image shown on the Inkplate display with color dithering.
 *
 * Notes:
 * - The entire PNG is loaded into heap before decoding.
 * - Inkplate 7 has PSRAM; large files (several MB) fit easily.
 * - Dithering is enabled by default (fifth argument to image.draw).
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
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "png_from_buffer";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(2);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setCursor(10, 10);
    display.print("SD card error!");
    display.display();
    return;
  }

  FILE *f = fopen("/sdcard/image.png", "rb");
  if (!f) {
    ESP_LOGE(TAG, "Cannot open image.png");
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
    ESP_LOGE(TAG, "Not enough RAM. Free heap: %lu",
             (unsigned long)esp_get_free_heap_size());
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

  ESP_LOGI(TAG, "Decoding PNG from buffer...");

  if (!display.image.draw(buf, (uint32_t)fileSize, 0, 0, true, false)) {
    ESP_LOGE(TAG, "PNG decode error");
    display.setCursor(10, 10);
    display.print("PNG decode error");
  }

  free(buf);
  display.display();

  ESP_LOGI(TAG, "Done");
}
