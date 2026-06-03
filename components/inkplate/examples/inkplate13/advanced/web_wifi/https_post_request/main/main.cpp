/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       WiFi HTTPS POST request example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates how to connect Inkplate 13SPECTRA to a WiFi
 *              network and send periodic HTTPS POST requests to webhook.site.
 *              This free online service allows real-time inspection of HTTP
 *              requests, making it useful for testing IoT data transmission.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      Stable WiFi connection, webhook.site URL
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Visit https://webhook.site and copy your unique webhook URL path.
 * 2) Paste the path (e.g. "/abcd-1234-efgh") into WEBHOOK_PATH below.
 * 3) Build and flash to Inkplate 13SPECTRA.
 * 4) Watch incoming POST requests live on webhook.site.
 *
 * Expected output:
 * - Inkplate display shows example information.
 * - webhook.site displays incoming POST requests every 20 seconds.
 *
 * Notes:
 * - Certificate verification is disabled (CONFIG_ESP_TLS_INSECURE=y).
 * - Replace WEBHOOK_PATH with your unique webhook.site path.
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
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "HTTPS_POST";

#define POSTING_INTERVAL_MS (20ULL * 1000ULL)
#define WEBHOOK_HOST        "webhook.site"
#define WEBHOOK_PATH        "/YOUR-UNIQUE-WEBHOOK-ID" // replace with your webhook.site path

static void sendPost() {
  char postData[64];
  int value = (int)(esp_random() % 40);
  snprintf(postData, sizeof(postData), "value=%d", value);

  char url[256];
  snprintf(url, sizeof(url), "https://%s%s", WEBHOOK_HOST, WEBHOOK_PATH);

  esp_http_client_config_t config = {};
  config.url = url;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = 10000;
  config.skip_cert_common_name_check = true;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    return;
  }

  esp_http_client_set_header(client, "Content-Type",
                             "application/x-www-form-urlencoded");
  esp_http_client_set_header(client, "User-Agent", "Inkplate-ESP32S3");
  esp_http_client_set_post_field(client, postData, strlen(postData));

  esp_err_t ret = esp_http_client_perform(client);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "POST sent: %s  status=%d", postData,
             esp_http_client_get_status_code(client));
  } else {
    ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(ret));
  }

  esp_http_client_cleanup(client);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
  display.setTextWrap(true);
  display.setTextSize(5);
  display.setCursor(0, 0);
  display.print("HTTPS POST example\n\nUsing webhook.site");
  display.display();

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    display.print("\nWiFi failed!");
    display.display();
    return;
  }

  ESP_LOGI(TAG, "WiFi connected");

  while (true) {
    sendPost();
    vTaskDelay(pdMS_TO_TICKS(POSTING_INTERVAL_MS));
  }
}
