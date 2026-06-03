/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Write a text file to microSD card on Soldered Inkplate 13SPECTRA.
 *
 * @details     Initialises the microSD card and writes a string to a file
 *              named test.txt in the card root. After writing, the SD card is
 *              put to sleep to save power. Results are logged via ESP-IDF.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable, microSD card
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Insert a FAT32-formatted microSD card.
 * 2) Build and flash to Inkplate 13SPECTRA.
 * 3) Check the serial log for success/failure; test.txt is created on the card.
 *
 * Expected output:
 * - "Write successful" in the serial log; test.txt created on the SD card.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Use the sd_txt_read example to verify the written file.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include <stdio.h>

#define FILE_PATH     "/sdcard/test.txt"
#define DATA_TO_WRITE "Hello from Inkplate 13SPECTRA!\n"

static const char *TAG = "sd_txt_write";

extern "C" void app_main(void) {
  Inkplate display;

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    return;
  }

  FILE *f = fopen(FILE_PATH, "w");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open %s for writing", FILE_PATH);
    display.sdCardSleep();
    return;
  }

  fprintf(f, DATA_TO_WRITE);
  fclose(f);
  ESP_LOGI(TAG, "Write successful: %s", FILE_PATH);

  display.sdCardSleep();
}
