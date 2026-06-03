/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Write a text file to SD card on Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to create and write a .txt file on a
 *              FAT-formatted SD card using POSIX file I/O on Inkplate 6 Flick.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable, microSD card
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Insert a FAT-formatted SD card into Inkplate 6 Flick.
 * 2) Build and flash.
 * 3) "test.txt" is created on the SD card with the defined message.
 * 4) Result shown on the e-paper display.
 *
 * Expected output:
 * - "Write successful!" shown on the display if file was created.
 * - Error message if SD init or file creation fails.
 *
 * Notes:
 * - Always close files after writing to prevent corruption.
 * - SD card is put to sleep after the operation.
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

static const char *TAG = "MAIN";
static const char *FILE_NAME = "/sdcard/test.txt";
static const char *DATA_TO_WRITE =
    "Hello! This is the file writing example for Inkplate 6 Flick.\n";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextSize(3);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.print("SD Card error!");
    display.partialUpdate();
    return;
  }

  display.print("SD Card OK!");
  display.partialUpdate();

  FILE *f = fopen(FILE_NAME, "w");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    display.clearDisplay();
    display.print("File create error!");
    display.display();
    display.sdCardSleep();
    return;
  }

  fprintf(f, "%s", DATA_TO_WRITE);
  fclose(f);
  ESP_LOGI(TAG, "Write successful");

  display.sdCardSleep();

  display.clearDisplay();
  display.setCursor(0, 300);
  display.print("Write successful!");
  display.display();
}
