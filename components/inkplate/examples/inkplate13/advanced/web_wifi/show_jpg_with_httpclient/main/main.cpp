/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download JPG into a buffer and display it (Inkplate 13SPECTRA).
 *
 * @details     Demonstrates how to manually download a JPG image into a PSRAM
 *              buffer using the WiFi download API, then draw it on the
 *              e-paper display from the buffer. This approach is useful when
 *              you need to pre-process the image data before displaying it.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Enter your WiFi credentials via menuconfig.
 * 2) Optionally change IMAGE_URL to any accessible JPG or PNG URL.
 * 3) Build and flash to Inkplate 13SPECTRA.
 * 4) Board connects to WiFi, downloads the image, and displays it.
 *
 * Expected output:
 * - JPG image downloaded and displayed on the Inkplate screen.
 *
 * Notes:
 * - Image is downloaded into PSRAM; ensure image size fits available memory.
 * - Certificate verification is disabled (CONFIG_ESP_TLS_INSECURE=y).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
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

static const char *TAG = "SHOW_JPG_HTTPCLIENT";

#define IMAGE_URL                                                              \
  "https://raw.githubusercontent.com/SolderedElectronics/"                    \
  "Inkplate-Arduino-library/master/examples/Inkplate10/Advanced/WEB_WiFi/"    \
  "Inkplate10_Show_JPG_With_HTTPClient/image.jpg"

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.display();
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
  display.setTextWrap(true);
  display.setCursor(0, 0);

  display.print("Connecting to WiFi...");
  display.display();

  display.wifi.begin();
  if (!display.wifi.waitForConnect()) {
    display.print("\nWiFi connection failed!");
    display.display();
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  display.print("\nWiFi OK! Downloading...");
  display.display();

  // Download image into buffer
  int32_t size = 0;
  uint8_t *buffer = display.wifi.downloadFileHTTPS(IMAGE_URL, &size);

  if (!buffer || size <= 0) {
    display.print("\nDownload failed!");
    display.display();
    ESP_LOGE(TAG, "Download failed, size=%ld", size);
    return;
  }

  ESP_LOGI(TAG, "Downloaded %ld bytes, drawing...", size);

  // Draw from buffer
  if (!display.image.draw(buffer, (uint32_t)size, 0, 0, true, false)) {
    display.print("\nImage draw error");
    display.display();
    ESP_LOGE(TAG, "Image draw failed");
  } else {
    display.display();
  }

  free(buffer);
}
