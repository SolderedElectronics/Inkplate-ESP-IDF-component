/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read a text file from microSD card on Soldered Inkplate 7.
 *
 * @details     Initialises the microSD card, reads up to 3000 characters from
 *              text.txt in the card root, and displays the content on the
 *              e-paper screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable, microSD card
 * - Extra:      A file named text.txt in the root of the microSD card
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Create or copy text.txt to the root of a FAT32-formatted microSD card.
 * 2) Insert the card, build and flash to Inkplate 7.
 * 3) The display shows the file contents.
 *
 * Expected output:
 * - Contents of text.txt displayed on the e-paper screen.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Only the first 3000 bytes of the file are read.
 * - Use the sd_txt_write example to create the test file.
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
#include <string.h>

#define FILE_PATH "/sdcard/text.txt"
#define MAX_CHARS 3000

static const char *TAG = "sd_txt_read";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(2);
  display.setCursor(0, 0);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.print("SD card error!");
    display.display();
    return;
  }

  display.print("SD Card OK! Reading file...");
  display.display();

  FILE *f = fopen(FILE_PATH, "r");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open %s", FILE_PATH);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("File open error!");
    display.display();
    display.sdCardSleep();
    return;
  }

  char text[MAX_CHARS + 1];
  size_t len = fread(text, 1, MAX_CHARS, f);
  fclose(f);
  display.sdCardSleep();

  text[len] = '\0';
  ESP_LOGI(TAG, "Read %zu bytes", len);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(text);
  display.display();
}
