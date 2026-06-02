/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       HTTPS POST request example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to connect Inkplate 4TEMPERA to a WiFi network
 *              and send periodic HTTPS POST requests with a JSON payload. The
 *              example uses the JSONPlaceholder fake REST API to test
 *              request/response flow and logs the HTTP status code and response
 *              body via ESP_LOG.
 *
 *              HTTPS is used without certificate validation (insecure mode) via
 *              the ESP-IDF HTTP client skip_cert_common_name_check flag.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Stable WiFi Internet connection
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 * - Menuconfig -> Component config -> ESP-TLS -> Allow potentially insecure
 *   options -> Skip server certificate verification by default (for demo use)
 *
 * How to use:
 * 1) Set your WiFi SSID and password in menuconfig.
 * 2) Build and flash to Inkplate 4TEMPERA.
 * 3) The board connects to WiFi and periodically sends an HTTPS POST request.
 * 4) Observe the returned status code and response via idf.py monitor.
 *
 * Expected output:
 * - Inkplate display shows a short message prompting to open serial monitor.
 * - Serial monitor shows WiFi connection status, HTTP status code, and response.
 *
 * Notes:
 * - skip_cert_common_name_check disables certificate validation. Not for
 *   production use — use a pinned certificate instead (see https_with_certificate
 *   example).
 * - JSONPlaceholder is a fake API: the response looks real, but data is not
 *   persisted.
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
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "HTTPS_POST";

#define API_URL "https://jsonplaceholder.typicode.com/posts"
#define DELAY_BETWEEN_REQUESTS_MS 10000

static char s_responseBuffer[2048];

static esp_err_t httpEventHandler(esp_http_client_event_t *evt) {
  if (evt->event_id == HTTP_EVENT_ON_DATA) {
    int remaining = sizeof(s_responseBuffer) - 1 -
                    strlen(s_responseBuffer);
    if (remaining > 0) {
      int copy = (evt->data_len < remaining) ? evt->data_len : remaining;
      strncat(s_responseBuffer, (char *)evt->data, copy);
    }
  }
  return ESP_OK;
}

static void sendHTTPSPost() {
  memset(s_responseBuffer, 0, sizeof(s_responseBuffer));

  static const char *postBody = "{\"title\": \"Hello Inkplate 4TEMPERA\"}";

  esp_http_client_config_t config = {};
  config.url = API_URL;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = 15000;
  config.skip_cert_common_name_check = true;
  config.event_handler = httpEventHandler;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    return;
  }

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, postBody, strlen(postBody));

  esp_err_t ret = esp_http_client_perform(client);
  if (ret == ESP_OK) {
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTPS POST status=%d", status);
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

  display.setTextSize(3);
  display.setCursor(20, 40);
  display.print("HTTPS POST example");
  display.setTextSize(2);
  display.setCursor(20, 120);
  display.print("Open serial monitor");
  display.setCursor(20, 150);
  display.print("(idf.py monitor) to");
  display.setCursor(20, 180);
  display.print("see what's happening.");
  display.display();

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    display.setTextSize(3);
    display.setCursor(20, 280);
    display.print("WiFi failed!");
    display.display();
    return;
  }

  ESP_LOGI(TAG, "WiFi connected");

  while (true) {
    sendHTTPSPost();
    vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_REQUESTS_MS));
  }
}
