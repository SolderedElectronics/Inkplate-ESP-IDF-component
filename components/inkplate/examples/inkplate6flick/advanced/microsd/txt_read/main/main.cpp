/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read and display text file from SD card on Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to open a .txt file from a FAT-formatted SD
 *              card and display its contents on the Inkplate 6 Flick e-paper
 *              display using POSIX file I/O.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable, microSD card
 * - Extra:      text.txt file on SD card root (up to 3000 characters)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Copy a file named text.txt to the root of a FAT-formatted SD card.
 * 2) Insert the SD card into Inkplate 6 Flick.
 * 3) Build and flash.
 * 4) File contents are displayed on the e-paper screen.
 *
 * Expected output:
 * - Contents of text.txt shown on the Inkplate display.
 *
 * Notes:
 * - Content is limited to 3000 characters in this example.
 * - SD card is put to sleep after reading to reduce power consumption.
 * - SD card is mounted at /sdcard.
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
#include <stdio.h>
#include <string.h>

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextSize(2);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.print("SD Card error!");
    display.partialUpdate();
    return;
  }

  display.print("SD Card OK! Reading...");
  display.partialUpdate();

  FILE *f = fopen("/sdcard/text.txt", "r");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open text.txt");
    display.clearDisplay();
    display.print("File open error");
    display.display();
    display.sdCardSleep();
    return;
  }

  char text[3001] = {};
  fread(text, 1, sizeof(text) - 1, f);
  fclose(f);
  ESP_LOGI(TAG, "Read: %s", text);

  display.sdCardSleep();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(text);
  display.display();
}
