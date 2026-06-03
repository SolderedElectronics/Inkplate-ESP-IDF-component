/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       HTTPS POST request example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to connect Inkplate 6 Flick to a WiFi network
 *              and send an HTTPS POST request with JSON data. The example uses
 *              the JSONPlaceholder test API as a simple endpoint and prints
 *              the HTTP status code and response to the serial monitor.
 *              Certificate validation is disabled (insecure mode) for simplicity.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Open Serial Monitor at 115200 baud.
 * 3) The device connects to WiFi and sends an HTTPS POST every 10 seconds.
 * 4) Serial Monitor shows HTTP status code and server response.
 *
 * Expected output:
 * - Inkplate display shows "HTTPS POST Request example" and serial monitor hint.
 * - Serial Monitor shows HTTP status code and JSON response from the server.
 *
 * Notes:
 * - This example uses CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY (set in
 *   sdkconfig.defaults), which disables certificate validation.
 *   For production use, validate the server certificate.
 * - JSONPlaceholder is a fake API for testing; it echoes data but does not
 *   persist it like a real backend.
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
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "HTTPS_POST";

#define DELAY_BETWEEN_REQUESTS_MS 10000
#define API_URL                   "https://jsonplaceholder.typicode.com/posts"

static char s_responseBuffer[1024];
static int  s_responseLen = 0;

static esp_err_t httpEventHandler(esp_http_client_event_t *evt) {
  if (evt->event_id == HTTP_EVENT_ON_DATA) {
    int copy = evt->data_len;
    if (s_responseLen + copy >= (int)sizeof(s_responseBuffer) - 1)
      copy = (int)sizeof(s_responseBuffer) - 1 - s_responseLen;
    if (copy > 0) {
      memcpy(s_responseBuffer + s_responseLen, evt->data, copy);
      s_responseLen += copy;
      s_responseBuffer[s_responseLen] = '\0';
    }
  }
  return ESP_OK;
}

static void sendPost() {
  const char *jsonBody = "{\"title\":\"Hello Inkplate\"}";

  s_responseLen          = 0;
  s_responseBuffer[0]    = '\0';

  esp_http_client_config_t config = {};
  config.url                      = API_URL;
  config.method                   = HTTP_METHOD_POST;
  config.timeout_ms               = 10000;
  config.event_handler            = httpEventHandler;
  config.skip_cert_common_name_check = true;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    return;
  }

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, jsonBody, strlen(jsonBody));

  esp_err_t ret = esp_http_client_perform(client);
  if (ret == ESP_OK) {
    int statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Status code: %d", statusCode);
    ESP_LOGI(TAG, "Response: %s", s_responseBuffer);
  } else {
    ESP_LOGE(TAG, "HTTPS POST failed: %s", esp_err_to_name(ret));
  }

  esp_http_client_cleanup(client);
}

extern "C" void app_main(void) {
  static Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.setTextWrap(true);

  display.setTextSize(5);
  display.setCursor(0, 0);
  display.print("HTTPS POST\nRequest example\n\n");
  display.setTextSize(4);
  display.print("Open Serial Monitor at\n115200 baud to see\nwhat's happening");
  display.display();

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  ESP_LOGI(TAG, "WiFi connected");

  while (true) {
    sendPost();
    vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_REQUESTS_MS));
  }
}
