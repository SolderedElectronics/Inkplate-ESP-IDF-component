/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       SD card image gallery example for Soldered Inkplate 6COLOR.
 *
 * @details     Initialises the microSD card, scans the root directory for image
 *              files (BMP, JPG, PNG), picks one at random, displays it on the
 *              7-color (black/white/green/blue/red/yellow/orange) e-paper
 *              screen, then enters deep sleep for 5 minutes.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6COLOR
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6COLOR, USB cable, microSD card
 * - Extra:      One or more BMP/JPG/PNG images in the microSD card root
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6COLOR
 *
 * How to use:
 * 1) Copy BMP, JPG, or PNG images to the root of a FAT32-formatted microSD card.
 * 2) Insert the card, build and flash to Inkplate 6COLOR.
 * 3) A random image is displayed each wake cycle.
 *
 * Expected output:
 * - A randomly selected image from the SD card displayed on the 600x448 color
 *   screen.
 * - Board enters deep sleep for 5 minutes then wakes and picks a new image.
 *
 * Notes:
 * - Deep sleep resets normal RAM; the whole program runs in app_main each wake.
 * - Hidden files and files without a supported extension are ignored.
 * - Adjust TIME_TO_SLEEP_US to change the refresh interval.
 * - Inkplate 6COLOR renders images using its 7-color palette (black, white,
 *   green, blue, red, yellow, orange), automatically selected at compile time
 *   by the board macro; no application-level changes are needed for the extra
 *   orange color compared to boards with fewer palette entries.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6COLOR in the boards menu."
#endif

#include "Inkplate.h"
#include "dirent.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "string.h"

#define TIME_TO_SLEEP_US (5ULL * 60ULL * 1000000ULL)
#define MAX_FILES        100
#define MAX_NAME_LEN     50

static const char *TAG = "gallery";

static int endsWith(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

extern "C" void app_main(void) {
  Inkplate display;

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    goto sleep;
  }

  {
    char files[MAX_FILES][MAX_NAME_LEN];
    int fileCount = 0;

    DIR *dir = opendir(display.getMountPoint());
    if (!dir) {
      ESP_LOGE(TAG, "Failed to open SD root");
      goto sleep;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && fileCount < MAX_FILES) {
      const char *name = entry->d_name;
      if (entry->d_type != DT_REG || name[0] == '.')
        continue;
      if (endsWith(name, ".bmp") || endsWith(name, ".jpg") ||
          endsWith(name, ".png")) {
        strncpy(files[fileCount], name, MAX_NAME_LEN - 1);
        files[fileCount][MAX_NAME_LEN - 1] = '\0';
        ESP_LOGI(TAG, "Found: %s", files[fileCount]);
        fileCount++;
      }
    }
    closedir(dir);

    if (fileCount == 0) {
      ESP_LOGW(TAG, "No image files found on SD card");
      goto sleep;
    }

    int chosen = esp_random() % fileCount;
    ESP_LOGI(TAG, "Drawing: %s", files[chosen]);
    display.image.draw(files[chosen], 0, 0, false, false);
    display.display();
  }

sleep:
  display.sdCardSleep();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}
