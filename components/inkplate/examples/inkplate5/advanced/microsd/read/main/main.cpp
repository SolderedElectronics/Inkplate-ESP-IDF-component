/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read and display text file from SD card on Soldered Inkplate 5.
 *
 * @details     Demonstrates how to open a .txt file from a FAT-formatted SD
 *              card and display its contents on the Inkplate 5 e-paper display.
 *              The example reads a file named "text.txt" from the SD card and
 *              prints its content on screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable, microSD card
 * - Extra:      text.txt file on SD card
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Copy a text file named "text.txt" to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 5.
 * 4) The file contents will be read and displayed on the e-paper screen.
 *
 * Expected output:
 * - The contents of text.txt are displayed on the Inkplate display.
 *
 * Notes:
 * - File name must be exactly "text.txt" for this example.
 * - SD card must be properly formatted (FAT/FAT32).
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
#include <string.h>

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    return;
  }

  FILE *f = fopen("/sdcard/text.txt", "r");
  char buf[1024] = {};

  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }
  fgets(buf, sizeof(buf), f);
  fclose(f);
  ESP_LOGI(TAG, "Read: %s", buf);

  display.sdCardSleep();

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(10, 10);
  display.print("From SD card:");
  display.setCursor(10, 60);
  display.print(buf);
  display.display();
}
