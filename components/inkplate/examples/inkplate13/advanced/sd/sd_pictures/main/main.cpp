/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display images from microSD card on Soldered Inkplate 13SPECTRA.
 *
 * @details     Initialises the microSD card and displays two JPEG images
 *              stored in the card root. Each image is shown for 5 seconds.
 *              Supports BMP, JPEG, and PNG files up to 1600x1200 pixels.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable, microSD card
 * - Extra:      picture1.jpg and picture2.jpg in the microSD card root
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Copy picture1.jpg and picture2.jpg to the root of a FAT32 microSD card.
 * 2) Insert the card, build and flash to Inkplate 13SPECTRA.
 * 3) Each image is displayed for 5 seconds in sequence.
 *
 * Expected output:
 * - picture1.jpg displayed, then picture2.jpg displayed.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Images should not exceed 1600x1200 pixels.
 * - Dithering is enabled (third argument = true).
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "sd_pictures";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(3);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setCursor(0, 0);
    display.print("SD card error!");
    display.display();
    return;
  }

  ESP_LOGI(TAG, "SD card OK, loading picture1.jpg");

  if (display.image.draw("picture1.jpg", 0, 0, true, false)) {
    display.display();
    vTaskDelay(pdMS_TO_TICKS(5000));
  } else {
    ESP_LOGE(TAG, "Failed to draw picture1.jpg");
  }

  display.clearDisplay();
  ESP_LOGI(TAG, "Loading picture2.jpg");

  if (display.image.draw("picture2.jpg", 0, 0, true, false)) {
    display.display();
    vTaskDelay(pdMS_TO_TICKS(5000));
  } else {
    ESP_LOGE(TAG, "Failed to draw picture2.jpg");
  }

  display.sdCardSleep();
}
