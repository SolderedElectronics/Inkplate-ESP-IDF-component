/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple HTTP web content fetch for Soldered Inkplate 7.
 *
 * @details     Demonstrates how to connect Inkplate 7 to a WiFi
 *              network, perform a basic HTTP request to retrieve data from the
 *              Internet, and display the received content on the e-paper
 *              display. This example does NOT parse HTML; it simply prints
 *              the raw HTTP response body on the screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Enter your WiFi SSID and password via menuconfig.
 * 2) Build and flash to Inkplate 7.
 * 3) The board connects to WiFi, fetches http://example.com, and displays it.
 *
 * Expected output:
 * - Raw HTML from example.com displayed on the Inkplate screen.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE7
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate7 in the boards menu."
#endif

#include "Inkplate.h"
#include "WiFi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "HTTP_REQUEST";

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
  display.setTextWrap(true);

  display.print("Connecting to WiFi...");
  display.display();

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    display.print("WiFi connection failed!");
    display.display();
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  display.print("Connected!");
  display.display();

  int32_t len = 32768;
  uint8_t *data = wifi.downloadFile("http://example.com/index.html", &len);

  if (!data || len <= 0) {
    display.print("HTTP request failed!");
    display.display();
    ESP_LOGE(TAG, "HTTP request failed");
    return;
  }

  data[len] = '\0';
  ESP_LOGI(TAG, "Received %ld bytes", len);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print((char *)data);
  display.display();

  free(data);
}
