/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       LAN image gallery for Soldered Inkplate 10.
 *
 * @details     Turns Inkplate 10 into a local network (LAN) image gallery.
 *              The board connects to a WiFi network and runs an HTTP server
 *              (main/webserver.cpp) that serves a small upload page from any
 *              browser on the same network. Uploaded pictures are written to
 *              the microSD card, and this file periodically scans the SD
 *              card root directory, picks a random picture, and renders it
 *              on the e-paper display in grayscale.
 *
 *              Supported formats are BMP and JPEG (JPG/JPEG). Basic image
 *              dimension detection is implemented for both formats (reading
 *              just the file header) so the picture can be centered on
 *              screen; if size detection fails the picture is drawn at
 *              (0, 0). Images rotate automatically every
 *              IMAGE_CHANGE_INTERVAL_MS, and immediately after a new upload
 *              finishes.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable, microSD card (FAT/FAT32 formatted)
 * - Extra:      WiFi network + a phone/PC with a web browser on the same LAN
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - SD card format: FAT / FAT32
 * - Set IMAGE_CHANGE_INTERVAL_MS below to control rotation timing
 *
 * How to use:
 * 1) Format a microSD card as FAT32 and insert it into Inkplate 10 (it can
 *    start out empty).
 * 2) Build and flash to Inkplate 10, then open the Serial Monitor.
 * 3) Once WiFi connects, the device's IP address is printed to the log (and
 *    shown briefly on the display).
 * 4) From a phone/PC on the same network, open http://<printed-ip>/ in a
 *    web browser.
 * 5) Pick an image file and press Upload. After the upload finishes, the
 *    picture list is rebuilt and a random picture (which may be the one
 *    just uploaded) is shown on the e-paper display.
 * 6) Pictures also rotate automatically every IMAGE_CHANGE_INTERVAL_MS.
 *
 * Expected output:
 * - E-paper display shows a randomly chosen picture from the SD card,
 *   centered, with a small footer label rendered on top.
 * - Serial Monitor prints WiFi connection progress, SD scan results,
 *   detected picture dimensions, and upload/write diagnostics.
 *
 * Notes:
 * - Display mode: grayscale (3-bit); partial updates are not available in
 *   grayscale mode, so every picture change is a full e-paper refresh.
 * - SD card access is shared between the HTTP upload handler
 *   (webserver.cpp, running in the httpd task) and the rotation loop below
 *   (running in the main task); a FreeRTOS mutex serializes SD reads/
 *   writes between the two.
 * - Only files in the SD card root directory are scanned; large SD card
 *   directories may slow down buildImageList() because it re-scans on
 *   every rotation and after every upload.
 * - JPEG/BMP decoding and buffering consume RAM; very large pictures may
 *   fail to decode or draw.
 * - This is a demo web server without authentication; avoid exposing it on
 *   untrusted networks.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "webserver.h"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "LAN_GALLERY";

// How often the displayed picture changes automatically (milliseconds).
#define IMAGE_CHANGE_INTERVAL_MS (30 * 1000UL)

// Maximum number of pictures tracked from the SD card root directory, and
// the maximum length of a tracked file name.
#define GALLERY_MAX_IMAGES 128
#define GALLERY_NAME_LEN 64

// Display instance shared with the HTTP handlers in webserver.cpp (via the
// startFileUpload()/writeFileData()/finishFileUpload() functions below).
// Handlers registered with esp_http_server are free functions, so the
// display and gallery state are kept as file-scope statics instead of
// locals in app_main().
static Inkplate display;

// Names (relative to the SD card root) of every supported picture found by
// the last buildImageList() scan.
static char s_imageNames[GALLERY_MAX_IMAGES][GALLERY_NAME_LEN];
static int s_numImages = 0;

// Serializes SD card access between the httpd task (uploads) and the main
// task (scanning the picture list and drawing).
static SemaphoreHandle_t s_sdMutex = nullptr;

// State for the upload currently being written to the SD card.
static FILE *s_uploadFile = nullptr;
static volatile bool s_uploadComplete = false;

// Device IP address, formatted once after WiFi connects, used in the
// on-screen footer.
static char s_ipStr[16] = "0.0.0.0";

/* -------------------------------------------------------------------------- */
/*                             Picture list helpers                          */
/* -------------------------------------------------------------------------- */

// Returns true if name ends with a supported picture extension (BMP/JPG/
// JPEG), case-insensitively.
static bool isGalleryImage(const char *name) {
  const char *dot = strrchr(name, '.');
  if (!dot)
    return false;
  return strcasecmp(dot, ".bmp") == 0 || strcasecmp(dot, ".jpg") == 0 ||
         strcasecmp(dot, ".jpeg") == 0;
}

// Scans the SD card root directory for supported picture files and fills
// s_imageNames. Must be called with s_sdMutex held.
static int buildImageList() {
  ESP_LOGI(TAG, "Building picture list...");

  DIR *dir = opendir(display.getMountPoint());
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open SD card root directory");
    s_numImages = 0;
    return 0;
  }

  int count = 0;
  struct dirent *entry;
  while (count < GALLERY_MAX_IMAGES && (entry = readdir(dir)) != nullptr) {
    const char *name = entry->d_name;
    if (entry->d_type != DT_REG || name[0] == '.')
      continue; // skip directories, hidden files, "." and ".."
    if (!isGalleryImage(name))
      continue;

    snprintf(s_imageNames[count], GALLERY_NAME_LEN, "%s", name);
    ESP_LOGI(TAG, "Found picture: %s", s_imageNames[count]);
    count++;
  }
  closedir(dir);

  s_numImages = count;
  ESP_LOGI(TAG, "Total pictures found: %d", s_numImages);
  return s_numImages;
}

// Picks a random entry from the current picture list. Returns false if the
// list is empty.
static bool pickRandomImageName(char *out, size_t outSize) {
  if (s_numImages <= 0)
    return false;

  int idx = (int)(esp_random() % (uint32_t)s_numImages);
  snprintf(out, outSize, "%s", s_imageNames[idx]);
  return true;
}

/* -------------------------------------------------------------------------- */
/*                     Lightweight BMP/JPEG size detection                   */
/* -------------------------------------------------------------------------- */

// Reads BMP width/height directly from the file header (offsets 18 and 22
// of the BITMAPINFOHEADER), without decoding the picture.
static bool readBmpSize(const char *path, int *w, int *h) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;

  uint8_t hdr[26];
  size_t n = fread(hdr, 1, sizeof(hdr), f);
  fclose(f);
  if (n < sizeof(hdr) || hdr[0] != 'B' || hdr[1] != 'M')
    return false;

  int32_t bw = (int32_t)(hdr[18] | (hdr[19] << 8) | (hdr[20] << 16) |
                         (hdr[21] << 24));
  int32_t bh = (int32_t)(hdr[22] | (hdr[23] << 8) | (hdr[24] << 16) |
                         (hdr[25] << 24));
  if (bw <= 0 || bh == 0)
    return false;

  *w = (int)bw;
  *h = (int)(bh < 0 ? -bh : bh);
  return true;
}

// Reads JPEG width/height by walking the marker segments until a
// start-of-frame (SOFn) marker is found, without decoding the picture.
static bool readJpegSize(const char *path, int *w, int *h) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;

  uint8_t soi[2];
  if (fread(soi, 1, 2, f) != 2 || soi[0] != 0xFF || soi[1] != 0xD8) {
    fclose(f);
    return false;
  }

  while (true) {
    // Find the next marker prefix byte (0xFF).
    int b;
    do {
      b = fgetc(f);
      if (b == EOF) {
        fclose(f);
        return false;
      }
    } while (b != 0xFF);

    // Skip fill bytes (extra 0xFF) to get the marker code.
    int marker;
    do {
      marker = fgetc(f);
      if (marker == EOF) {
        fclose(f);
        return false;
      }
    } while (marker == 0xFF);

    if (marker == 0xD9) // EOI, no SOF found
      break;
    if (marker == 0xD8 || marker == 0x01) // stray SOI/TEM, no payload
      continue;

    int lenHi = fgetc(f);
    int lenLo = fgetc(f);
    if (lenHi == EOF || lenLo == EOF) {
      fclose(f);
      return false;
    }
    int segLen = (lenHi << 8) | lenLo;
    if (segLen < 2) {
      fclose(f);
      return false;
    }

    bool isSOF = (marker >= 0xC0 && marker <= 0xC3) ||
                 (marker >= 0xC5 && marker <= 0xC7) ||
                 (marker >= 0xC9 && marker <= 0xCB) ||
                 (marker >= 0xCD && marker <= 0xCF);

    if (isSOF) {
      uint8_t sof[5]; // precision, height (2 bytes), width (2 bytes)
      if (fread(sof, 1, sizeof(sof), f) != sizeof(sof)) {
        fclose(f);
        return false;
      }
      *h = (sof[1] << 8) | sof[2];
      *w = (sof[3] << 8) | sof[4];
      fclose(f);
      return (*w > 0 && *h > 0);
    }

    // Not a SOF segment, skip the rest of it.
    if (fseek(f, segLen - 2, SEEK_CUR) != 0) {
      fclose(f);
      return false;
    }
  }

  fclose(f);
  return false;
}

// Detects a picture's width/height from its file extension. Returns false
// (leaving *w/*h untouched) if the format isn't recognised or reading
// fails.
static bool getImageSize(const char *name, int *w, int *h) {
  char path[96];
  snprintf(path, sizeof(path), "%s/%s", display.getMountPoint(), name);

  const char *dot = strrchr(name, '.');
  if (!dot)
    return false;

  if (strcasecmp(dot, ".bmp") == 0)
    return readBmpSize(path, w, h);
  if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0)
    return readJpegSize(path, w, h);

  return false;
}

/* -------------------------------------------------------------------------- */
/*                              Display helpers                              */
/* -------------------------------------------------------------------------- */

// Draws the named picture (relative to the SD card root) centered on the
// e-paper display, with a small footer label on top, then refreshes the
// screen. Must be called with s_sdMutex held.
static void showImage(const char *name) {
  ESP_LOGI(TAG, "Displaying: %s", name);
  display.clearDisplay();

  // Fall back to the full screen size (i.e. draw at 0,0) if the header
  // can't be parsed; getImageSize() only overwrites detectedW/detectedH
  // once it has fully validated the dimensions it read.
  int detectedW = 0, detectedH = 0;
  bool okSize = getImageSize(name, &detectedW, &detectedH);
  int imgW = okSize ? detectedW : display.width();
  int imgH = okSize ? detectedH : display.height();
  ESP_LOGI(TAG, "Picture size: %dx%d (detected=%s)", imgW, imgH,
           okSize ? "yes" : "no");

  int x = (display.width() - imgW) / 2;
  int y = (display.height() - imgH) / 2;
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  // display.image.draw() prefixes relative paths with the SD card mount
  // point automatically, so pass the bare file name here.
  if (!display.image.draw(name, x, y, true, false)) {
    ESP_LOGE(TAG, "Failed to draw picture: %s", name);
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(100, 300);
    display.print("Failed to load picture:");
    display.setCursor(100, 330);
    display.print(name);
  }

  // Footer label (white text on a black box) showing where to upload more
  // pictures from.
  char overlayText[64];
  snprintf(overlayText, sizeof(overlayText), "Inkplate LAN Gallery - http://%s/",
           s_ipStr);

  display.setTextSize(1);
  int16_t textW = (int16_t)(strlen(overlayText) * 6);
  int16_t textH = 10;
  int16_t padding = 6;

  int16_t boxW = textW + padding * 2;
  int16_t boxH = textH + padding * 2;
  int16_t boxX = display.width() - boxW;
  int16_t boxY = display.height() - boxH;

  display.fillRect(boxX, boxY, boxW, boxH, BLACK);
  display.setTextColor(WHITE);
  display.setCursor(boxX + padding, boxY + textH - 2);
  display.print(overlayText);

  display.display();
}

// Shows a simple full-screen message (used when there's nothing to draw).
static void showMessage(const char *line1, const char *line2) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(20, 40);
  display.print(line1);
  if (line2) {
    display.setCursor(20, 90);
    display.print(line2);
  }
  display.display();
}

// Rebuilds the picture list and shows a random picture (or a "no pictures"
// message if the SD card root is empty). Takes s_sdMutex internally.
static void refreshGallery() {
  if (xSemaphoreTake(s_sdMutex, portMAX_DELAY) == pdTRUE) {
    if (buildImageList() > 0) {
      char name[GALLERY_NAME_LEN];
      if (pickRandomImageName(name, sizeof(name)))
        showImage(name);
    } else {
      showMessage("No pictures found on SD!", "Upload one from your browser.");
    }
    xSemaphoreGive(s_sdMutex);
  }
}

/* -------------------------------------------------------------------------- */
/*               Upload helpers, called from webserver.cpp                   */
/* -------------------------------------------------------------------------- */

// Opens filename (relative to the SD card root) for writing, closing any
// previously open upload file first.
void startFileUpload(const char *filename) {
  if (xSemaphoreTake(s_sdMutex, portMAX_DELAY) == pdTRUE) {
    char path[96];
    snprintf(path, sizeof(path), "%s/%s", display.getMountPoint(), filename);

    if (s_uploadFile) {
      fclose(s_uploadFile);
      s_uploadFile = nullptr;
    }

    s_uploadFile = fopen(path, "wb");
    ESP_LOGI(TAG, "Opening %s: %s", path, s_uploadFile ? "SUCCESS" : "FAILED");
    xSemaphoreGive(s_sdMutex);
  }
}

// Writes a chunk of the uploaded picture to the currently open file.
void writeFileData(const uint8_t *data, size_t len) {
  if (xSemaphoreTake(s_sdMutex, portMAX_DELAY) == pdTRUE) {
    if (s_uploadFile) {
      size_t written = fwrite(data, 1, len, s_uploadFile);
      ESP_LOGI(TAG, "Wrote %u/%u bytes", (unsigned)written, (unsigned)len);
    } else {
      ESP_LOGE(TAG, "Upload file not open!");
    }
    xSemaphoreGive(s_sdMutex);
  }
}

// Closes the upload file and flags the main loop to rebuild the picture
// list and show a new picture.
void finishFileUpload() {
  if (xSemaphoreTake(s_sdMutex, portMAX_DELAY) == pdTRUE) {
    if (s_uploadFile) {
      fflush(s_uploadFile);
      fclose(s_uploadFile);
      s_uploadFile = nullptr;
      ESP_LOGI(TAG, "Upload file closed and flushed");
    }
    s_uploadComplete = true;
    xSemaphoreGive(s_sdMutex);
  }
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */

extern "C" void app_main(void) {
  display.setDisplayMode(GRAYSCALE); // 3-bit grayscale, matches original sketch
  showMessage("Connecting Wi-Fi...", nullptr);

  // Connect to WiFi using the credentials configured via menuconfig.
  display.wifi.begin();
  if (!display.wifi.waitForConnect()) {
    ESP_LOGE(TAG, "Failed to connect to WiFi, check menuconfig credentials");
  }

  // Fetch the IP address assigned to the station interface for the footer
  // label and the log line below.
  esp_netif_t *staNetif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (staNetif != nullptr) {
    esp_netif_ip_info_t ipInfo;
    if (esp_netif_get_ip_info(staNetif, &ipInfo) == ESP_OK) {
      snprintf(s_ipStr, sizeof(s_ipStr), IPSTR, IP2STR(&ipInfo.ip));
      ESP_LOGI(TAG, "Connected! Open http://%s/ in your browser", s_ipStr);
    }
  }

  s_sdMutex = xSemaphoreCreateMutex();

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    showMessage("SD card init failed!", "Insert a FAT32 microSD card and reset.");
  }

  // Start the LAN web server; uploads are written to the SD card via
  // startFileUpload()/writeFileData()/finishFileUpload() above.
  setupWebServer();

  // Build the picture list and show a random picture at startup.
  refreshGallery();

  TickType_t lastChange = xTaskGetTickCount();
  const TickType_t interval = pdMS_TO_TICKS(IMAGE_CHANGE_INTERVAL_MS);

  while (true) {
    if (s_uploadComplete) {
      s_uploadComplete = false;
      ESP_LOGI(TAG, "Upload complete, rebuilding picture list...");
      refreshGallery();
      lastChange = xTaskGetTickCount();
    } else if (xTaskGetTickCount() - lastChange >= interval) {
      refreshGallery();
      lastChange = xTaskGetTickCount();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
