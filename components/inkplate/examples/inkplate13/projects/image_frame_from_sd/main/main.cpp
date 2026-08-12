/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       SD card image slideshow ("digital picture frame") for Soldered
 *              Inkplate 13SPECTRA.
 *
 * @details     Cycles through image files stored in a folder on a microSD
 *              card, drawing one image, then deep-sleeping for
 *              SECS_BETWEEN_PICTURES before waking up (by timer or by the
 *              wake button) to show the next one. The image index is kept in
 *              RTC memory so the slideshow resumes where it left off across
 *              deep sleep cycles, wrapping back to the first image after the
 *              last one has been shown.
 *
 *              Supports BMP (1/4/8/24-bit), JPEG and PNG files. Images that
 *              can't be decoded are skipped automatically. Each image is
 *              dithered down to the panel's native color palette.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable, microSD card loaded with
 *               images
 * - SD card:    standard FAT32 format
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - In main/main.cpp, adjust FOLDER_PATH and SECS_BETWEEN_PICTURES below if
 *   needed.
 *
 * How to use:
 * 1) Copy your images into a folder on the SD card (default: "images", i.e.
 *    a folder named "images" at the card root) and insert the card.
 * 2) Build and flash to Inkplate 13SPECTRA.
 * 3) Images are shown one after another; press the wake button to skip to
 *    the next one without waiting for the timer.
 *
 * Expected output:
 * - Each image in the folder displayed in turn, one per wake cycle, dithered
 *   to the 6-color palette.
 * - "The folder is empty" if the folder has no files.
 * - "Error opening folder!" if FOLDER_PATH doesn't exist on the card.
 * - "SD Card error!" if the microSD card can't be initialised.
 *
 * Notes:
 * - Inkplate 13SPECTRA supports 6 colors: black, white, yellow, red, blue,
 *   green (no orange, unlike Inkplate 6Color). display.image.draw() dithers
 *   each image down to this palette.
 * - Deep sleep resets normal RAM; only nextImageIndex (RTC_DATA_ATTR)
 *   survives across cycles.
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
#include "dirent.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "stdint.h"
#include "string.h"

/******************CHANGE HERE***********************/

// Set the time between changing 2 images in seconds. Note: it will take a
// couple of seconds more than this (loading + drawing time) before the next
// image actually appears on the screen.
#define SECS_BETWEEN_PICTURES 60

// Folder on the SD card that contains the images, relative to the card
// root, with no leading or trailing slash.
#define FOLDER_PATH "images"

/****************************************************/

// GPIO tied to the wake-up button, used to skip to the next image early
// instead of waiting for the SECS_BETWEEN_PICTURES timer (matches the
// wake_up_button advanced example for this board).
#define WAKEUP_GPIO GPIO_NUM_18

// Upper bound on how many files this example will track per folder, and on
// how long a single file name can be. Both are static arrays (not on the
// stack) so raising these only costs .bss, not stack space.
#define MAX_FILES 300
#define MAX_NAME_LEN 128

static const char *TAG = "image_frame_from_sd";

// Index (within the directory listing built below) of the next image to
// show. Kept in RTC memory so the slideshow resumes where it left off after
// each deep sleep cycle instead of restarting from the first image.
RTC_DATA_ATTR static uint32_t nextImageIndex = 0;

// File names found directly under FOLDER_PATH, in the order the SD card's
// FAT directory returns them (not sorted - same as the original Arduino
// sketch, which also iterated files in raw on-disk directory order via
// SdFat rather than alphabetically).
static char files[MAX_FILES][MAX_NAME_LEN];

/**
 * @brief     Power down the SD card, arm the requested wakeup sources, and
 *            enter deep sleep. Never returns.
 *
 * @param     display        Inkplate instance, used to power down the SD
 *                            card before sleeping.
 * @param     enableTimer     true to also wake up after SECS_BETWEEN_PICTURES
 *                            (normal slideshow advance); false to only wake
 *                            up on the wake button (error / empty-folder
 *                            states, where retrying on a timer would just
 *                            repeat the same failure).
 */
[[noreturn]] static void deepSleep(Inkplate &display, bool enableTimer) {
  display.sdCardSleep();

  if (enableTimer) {
    esp_sleep_enable_timer_wakeup((uint64_t)SECS_BETWEEN_PICTURES * 1000000ULL);
  }
  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);

  esp_deep_sleep_start();
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(2);

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setCursor(10, 10);
    display.print("SD Card error!");
    display.display();
    deepSleep(display, false); // Wait for the wake button; retrying on a timer
                               // won't fix a missing/broken card.
  }

  // Open the folder and collect the names of the regular files directly
  // inside it (skipping subdirectories and hidden/dotfiles, matching the
  // original sketch's isSubDir()/isHidden() checks).
  char dirPath[64];
  snprintf(dirPath, sizeof(dirPath), "%s/%s", display.getMountPoint(),
           FOLDER_PATH);

  DIR *dir = opendir(dirPath);
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open folder: %s", dirPath);
    display.setCursor(10, 10);
    display.printf(
        "Error opening folder!\nMake sure that %s\nexists on the SD card.",
        FOLDER_PATH);
    display.display();
    deepSleep(display, false);
  }

  int fileCount = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL && fileCount < MAX_FILES) {
    if (entry->d_type == DT_DIR)
      continue; // Skip subdirectories.
    if (entry->d_name[0] == '.')
      continue; // Skip hidden files.

    strncpy(files[fileCount], entry->d_name, MAX_NAME_LEN - 1);
    files[fileCount][MAX_NAME_LEN - 1] = '\0';
    fileCount++;
  }
  closedir(dir);

  if (fileCount == 0) {
    ESP_LOGW(TAG, "No files found in %s", dirPath);
    display.setCursor(10, 10);
    display.print("The folder is empty");
    display.display();
    deepSleep(display, false);
  }

  // The folder's contents may have changed since the last cycle (files
  // added/removed); clamp a stale index back into range instead of reading
  // past the end of the freshly-built list.
  if (nextImageIndex >= (uint32_t)fileCount)
    nextImageIndex = 0;

  // Try images starting at nextImageIndex, wrapping around the folder.
  // Files that fail to draw (unsupported/corrupt) are skipped automatically,
  // matching the original sketch's "it will skip images that can't be
  // drawn" behavior. Bounded to fileCount attempts so a folder full of
  // undrawable files can't loop forever and drain the battery.
  bool shown = false;
  char imagePath[32 + MAX_NAME_LEN];
  for (int attempt = 0; attempt < fileCount; attempt++) {
    uint32_t idx = nextImageIndex;
    nextImageIndex = (nextImageIndex + 1) % (uint32_t)fileCount;

    // Relative path (no leading '/'): display.image.draw() prepends the SD
    // card mount point for relative paths, so this resolves to
    // "<mount>/FOLDER_PATH/<file>" without hardcoding the mount point here.
    snprintf(imagePath, sizeof(imagePath), "%s/%s", FOLDER_PATH, files[idx]);

    ESP_LOGI(TAG, "Drawing: %s", imagePath);
    display.clearDisplay(); // Clear any partial draw left by a previous failed
                            // attempt.
    if (display.image.draw(imagePath, 0, 0, true, false)) {
      display.display();
      shown = true;
      break;
    }

    ESP_LOGW(TAG, "Skipping file that couldn't be drawn: %s", imagePath);
  }

  if (!shown) {
    ESP_LOGE(TAG, "No drawable images found in %s", dirPath);
    display.clearDisplay();
    display.setCursor(10, 10);
    display.print("No drawable images found");
    display.display();
    deepSleep(display, false);
  }

  // An image is now on screen - sleep until it's time for the next one (or
  // the wake button is pressed to skip ahead).
  deepSleep(display, true);
}
