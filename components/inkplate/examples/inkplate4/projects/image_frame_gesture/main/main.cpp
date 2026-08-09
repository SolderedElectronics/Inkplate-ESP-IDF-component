/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Gesture-controlled image frame slideshow for Soldered
 *              Inkplate 4TEMPERA.
 *
 * @details     Implements an image-frame slideshow on Inkplate 4TEMPERA.
 *              Images are loaded from a folder on a FAT-formatted microSD
 *              card and rendered to the e-paper display in 3-bit grayscale
 *              mode.
 *
 *              Navigation is controlled by the onboard APDS9960 gesture
 *              sensor: a LEFT swipe advances to the next image and a RIGHT
 *              swipe goes back to the previous one. The gesture sensor is
 *              polled continuously in the main loop (this example does not
 *              use interrupts or deep sleep, to keep it consistent with the
 *              rest of the ported examples for this board).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable, microSD card
 * - Extra:      microSD card (FAT/FAT32) with image files in a folder
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 * - SD card format: FAT / FAT32
 * - Set IMAGE_FOLDER below to the folder on the SD card that holds the
 *   images (path is relative to the SD card root and must end with '/')
 *
 * How to use:
 * 1) Format a microSD card as FAT/FAT32.
 * 2) Create a folder (e.g. "images/") and copy your images into it.
 * 3) Set IMAGE_FOLDER below to match your folder name.
 * 4) Build and flash to Inkplate 4TEMPERA, then insert the SD card.
 * 5) The first image found is displayed; swipe LEFT/RIGHT over the
 *    APDS9960 sensor area to change images.
 *
 * Expected output:
 * - E-paper: one image displayed full-screen. A LEFT gesture advances to
 *   the next image, a RIGHT gesture goes back to the previous one.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (8 levels). Partial update is not
 *   available in grayscale mode, so every image change uses a full
 *   e-paper refresh.
 * - Supported image types depend on the Inkplate image decoder (BMP,
 *   JPEG, PNG). Files are matched by extension; anything else in the
 *   folder is ignored.
 * - Maximum of 512 images per folder (static index array size).
 * - Gesture sensitivity is set to the lowest gain to reduce accidental
 *   triggers.
 * - This example uses polling (not interrupts) to read gestures, matching
 *   the approach used in the APDS9960 sensor example for this board.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

/******************CHANGE HERE***********************/

// TODO: fill in your image folder here.
// Path to the folder with pictures on the microSD card, relative to the SD
// card root (e.g. there is a folder called "images" on the SD card).
// NOTE: Must end with '/'.
#define IMAGE_FOLDER "images/"

/****************************************************/

// Maximum number of images tracked per folder.
#define MAX_IMAGES 512
// How often the APDS9960 is polled for a new gesture.
#define GESTURE_POLL_INTERVAL_MS 200

static const char *TAG = "IMAGE_FRAME_GESTURE";

// Names of all supported image files found in IMAGE_FOLDER.
static char s_imageNames[MAX_IMAGES][64];
static int s_numImages = 0;
static int s_currentIndex = 0;

enum GestureDirection { GESTURE_NONE, GESTURE_NEXT, GESTURE_PREV };

// Returns true if the filename has a supported image extension.
static bool isImageFile(const char *name) {
  const char *dot = strrchr(name, '.');
  if (!dot)
    return false;
  return strcasecmp(dot, ".bmp") == 0 || strcasecmp(dot, ".jpg") == 0 ||
         strcasecmp(dot, ".jpeg") == 0 || strcasecmp(dot, ".png") == 0;
}

// Scans folderFullPath (an absolute path, including the SD mount point) and
// fills s_imageNames with the names of every supported image file found.
// Hidden files (name starting with '.') and non-image files are skipped.
static int scanImageFolder(const char *folderFullPath) {
  DIR *dir = opendir(folderFullPath);
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open folder: %s", folderFullPath);
    return 0;
  }

  int count = 0;
  struct dirent *entry;
  while (count < MAX_IMAGES && (entry = readdir(dir)) != nullptr) {
    if (entry->d_name[0] == '.')
      continue; // Skip hidden files, "." and ".."
    if (!isImageFile(entry->d_name))
      continue; // Skip anything that isn't a supported image type

    snprintf(s_imageNames[count], sizeof(s_imageNames[count]), "%s",
              entry->d_name);
    count++;
  }
  closedir(dir);

  return count;
}

// Draws the image at s_currentIndex on the screen (or an error message if
// there are no images / the image can't be loaded), then refreshes the
// e-paper display.
static void showCurrentImage(Inkplate &display) {
  display.clearDisplay();

  if (s_numImages == 0) {
    display.setCursor(10, 10);
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.print("No images found in folder:");
    display.setCursor(10, 40);
    display.print(IMAGE_FOLDER);
    display.display();
    return;
  }

  // Path relative to the SD card root; display.image.draw() prefixes it
  // with the SD card mount point.
  char imagePath[96];
  snprintf(imagePath, sizeof(imagePath), "%s%s", IMAGE_FOLDER,
            s_imageNames[s_currentIndex]);

  if (!display.image.draw(imagePath, 0, 0, true, false)) {
    ESP_LOGE(TAG, "Failed to draw image: %s", imagePath);
    display.setCursor(10, 10);
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.print("Failed to load image:");
    display.setCursor(10, 40);
    display.print(s_imageNames[s_currentIndex]);
  }

  display.display();
}

// Polls the APDS9960 for a pending gesture and translates it into a
// navigation direction. Only LEFT/RIGHT swipes are used; every other
// gesture (up/down/near/far/none) is ignored.
static GestureDirection pollGesture(Inkplate &display) {
  if (!display.apds.isGestureAvailable())
    return GESTURE_NONE;

  switch (display.apds.readGesture()) {
  case DIR_LEFT:
    return GESTURE_NEXT;
  case DIR_RIGHT:
    return GESTURE_PREV;
  default:
    return GESTURE_NONE;
  }
}

// Moves s_currentIndex according to the detected gesture (wrapping around
// at either end of the list) and redraws the display.
static void advanceImage(Inkplate &display, GestureDirection direction) {
  if (s_numImages == 0)
    return;

  if (direction == GESTURE_NEXT) {
    s_currentIndex++;
    if (s_currentIndex >= s_numImages)
      s_currentIndex = 0;
  } else if (direction == GESTURE_PREV) {
    s_currentIndex--;
    if (s_currentIndex < 0)
      s_currentIndex = s_numImages - 1;
  }

  showCurrentImage(display);
}

extern "C" void app_main(void) {
  static Inkplate display;

  display.setDisplayMode(GRAYSCALE); // 3-bit grayscale mode

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.clearDisplay();
    display.setCursor(10, 10);
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.print("SD Card error!");
    display.display();
    return;
  }

  // Enable the onboard APDS9960 gesture sensor. Gain is set to the lowest
  // value to reduce accidental triggers, matching the original example.
  display.apds.enableGestureSensor();
  display.apds.setGestureGain(0);

  // Build the absolute folder path (SD mount point + IMAGE_FOLDER) and scan
  // it for supported image files.
  char folderFullPath[96];
  snprintf(folderFullPath, sizeof(folderFullPath), "%s/%s",
            display.getMountPoint(), IMAGE_FOLDER);
  s_numImages = scanImageFolder(folderFullPath);
  ESP_LOGI(TAG, "Found %d image(s) in %s", s_numImages, folderFullPath);

  s_currentIndex = 0;
  showCurrentImage(display);

  while (true) {
    GestureDirection direction = pollGesture(display);
    if (direction != GESTURE_NONE)
      advanceImage(display, direction);

    vTaskDelay(pdMS_TO_TICKS(GESTURE_POLL_INTERVAL_MS));
  }
}
