/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       OpenAI text prompt example for Soldered Inkplate 6Color.
 *
 * @details     Connects Inkplate 6Color to WiFi, sends a text prompt to the
 *              OpenAI Chat Completions API over HTTPS, and renders the
 *              text response on the e-paper screen using a custom font
 *              (FreeMonoBold18pt7b).
 *
 *              The HTTPS POST request (JSON body + Authorization header) is
 *              built with esp_http_client using the low-level
 *              open/write/fetch_headers/read sequence. The request body is
 *              built with cJSON so the prompt text is properly escaped, and
 *              the response is parsed with cJSON to pull out
 *              choices[0].message.content.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      WiFi (2.4 GHz) connection + Internet access, OpenAI API key
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6Color
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - OPENAI_API_KEY and PROMPT_TEXT below -> fill in your own values
 *
 * How to use:
 * 1) Set OPENAI_API_KEY to a valid OpenAI API key.
 * 2) Set PROMPT_TEXT to whatever you want to ask the model.
 * 3) Configure WiFi credentials in menuconfig.
 * 4) Build and flash to Inkplate 6Color, then open the Serial Monitor.
 * 5) The device connects to WiFi, sends the prompt to OpenAI, and displays
 *    the returned text.
 *
 * Expected output:
 * - Display: the OpenAI text response, wrapped and rendered with the
 *   FreeMonoBold18pt7b font.
 * - On failure: an error message on the display ("WiFi failed!" or
 *   "OpenAI request failed.") plus details in the Serial Monitor log.
 *
 * Notes:
 * - This is a one-shot example: connect, send the prompt, display the
 *   response, done. The original Arduino sketch also fetched live weather
 *   data from Open-Meteo to build the prompt automatically and then entered
 *   deep sleep to repeat periodically; both are dropped here to keep the
 *   port focused on the OpenAI request/response/display flow. PROMPT_TEXT
 *   is a fixed compile-time placeholder instead.
 * - api.openai.com is signed by a well-known public CA, so this example
 *   verifies the server certificate using the ESP-IDF certificate bundle
 *   (esp_crt_bundle_attach) rather than pinning a single certificate like
 *   the https_with_certificate example does. That requires
 *   CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y (set in sdkconfig.defaults) and
 *   REQUIRES "mbedtls" in main/CMakeLists.txt.
 * - CONFIG_ESP_TLS_INSECURE / CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY are kept
 *   enabled in sdkconfig.defaults for parity with the other ported examples,
 *   but they are not used here: esp_http_client_config_t.crt_bundle_attach
 *   is set, so the connection is still verified against the CA bundle.
 * - Inkplate 6Color is a 7-color e-paper board (600x448), so this port uses
 *   INKPLATE_BLACK / INKPLATE_WHITE instead of the generic BLACK/WHITE
 *   macros, and there is no setDisplayMode() call since this board does not
 *   have selectable 1-bit/3-bit display modes like the monochrome boards.
 * - Protect your API key: do not commit a real key to a public repository.
 *   OpenAI API usage and quotas apply.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6Color in the boards menu."
#endif

#include "Inkplate.h"
#include "FreeMonoBold18pt7b.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "OPENAI_PROMPT";

// TODO: fill in your OpenAI API key (https://platform.openai.com/api-keys)
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"

// TODO: fill in the prompt you want to send to OpenAI
#define PROMPT_TEXT "Give me a one sentence fun fact about e-paper displays."

#define OPENAI_MODEL "gpt-4o-mini"
#define OPENAI_URL "https://api.openai.com/v1/chat/completions"
#define HTTP_TIMEOUT_MS 30000
#define OPENAI_RESPONSE_BUFFER_SIZE 8192
#define OPENAI_REPLY_MAX_LEN 1024

/**
 * Send a prompt to OpenAI's Chat Completions API and extract the reply text.
 *
 * Builds the JSON request body with cJSON, POSTs it to OPENAI_URL with an
 * Authorization: Bearer header using the esp_http_client
 * open/write/fetch_headers/read sequence, then parses the JSON response with
 * cJSON to pull out choices[0].message.content.
 *
 * @param prompt Prompt text to send as the user message.
 * @param outText Buffer that receives the reply text on success.
 * @param outTextSize Size of outText in bytes.
 * @return true on success (outText contains the reply), false on error.
 */
static bool requestOpenAI(const char *prompt, char *outText,
                          size_t outTextSize) {
  // Build request body: {"model": "...", "messages": [{"role": "user", "content": prompt}]}
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "model", OPENAI_MODEL);
  cJSON *messages = cJSON_AddArrayToObject(root, "messages");
  cJSON *message = cJSON_CreateObject();
  cJSON_AddStringToObject(message, "role", "user");
  cJSON_AddStringToObject(message, "content", prompt);
  cJSON_AddItemToArray(messages, message);

  char *requestBody = cJSON_PrintUnformatted(root);
  cJSON_Delete(root); // Frees "messages"/"message" too; requestBody is a
                      // separately allocated string.
  if (!requestBody) {
    ESP_LOGE(TAG, "Failed to serialize request JSON");
    return false;
  }
  int requestLen = (int)strlen(requestBody);

  esp_http_client_config_t config = {};
  config.url = OPENAI_URL;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = HTTP_TIMEOUT_MS;
  config.crt_bundle_attach = esp_crt_bundle_attach;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    cJSON_free(requestBody);
    return false;
  }

  char authHeader[160];
  snprintf(authHeader, sizeof(authHeader), "Bearer %s", OPENAI_API_KEY);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_header(client, "Authorization", authHeader);

  bool success = false;
  char *responseBuffer = nullptr;

  esp_err_t err = esp_http_client_open(client, requestLen);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
  } else if (esp_http_client_write(client, requestBody, requestLen) !=
             requestLen) {
    ESP_LOGE(TAG, "Failed to write full request body");
    esp_http_client_close(client);
  } else {
    int64_t contentLength = esp_http_client_fetch_headers(client);
    int statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "OpenAI HTTP status=%d content-length=%lld", statusCode,
             (long long)contentLength);

    responseBuffer = (char *)malloc(OPENAI_RESPONSE_BUFFER_SIZE);
    if (!responseBuffer) {
      ESP_LOGE(TAG, "Failed to allocate response buffer");
      esp_http_client_close(client);
    } else {
      int totalRead = 0;
      int r;
      while (totalRead < OPENAI_RESPONSE_BUFFER_SIZE - 1 &&
             (r = esp_http_client_read(client, responseBuffer + totalRead,
                                        OPENAI_RESPONSE_BUFFER_SIZE - 1 -
                                            totalRead)) > 0) {
        totalRead += r;
      }
      responseBuffer[totalRead] = '\0';
      esp_http_client_close(client);

      if (statusCode != 200) {
        ESP_LOGE(TAG, "OpenAI request failed (status %d): %s", statusCode,
                 responseBuffer);
      }

      cJSON *responseJson = cJSON_Parse(responseBuffer);
      if (!responseJson) {
        ESP_LOGE(TAG, "Failed to parse OpenAI response JSON");
      } else {
        cJSON *errorObj = cJSON_GetObjectItem(responseJson, "error");
        if (errorObj) {
          cJSON *errMsg = cJSON_GetObjectItem(errorObj, "message");
          ESP_LOGE(TAG, "OpenAI error: %s",
                   (errMsg && cJSON_IsString(errMsg))
                       ? cJSON_GetStringValue(errMsg)
                       : "unknown error");
        } else {
          cJSON *choices = cJSON_GetObjectItem(responseJson, "choices");
          cJSON *firstChoice = (choices && cJSON_IsArray(choices))
                                   ? cJSON_GetArrayItem(choices, 0)
                                   : nullptr;
          cJSON *messageObj =
              firstChoice ? cJSON_GetObjectItem(firstChoice, "message")
                          : nullptr;
          cJSON *contentObj =
              messageObj ? cJSON_GetObjectItem(messageObj, "content")
                        : nullptr;

          if (contentObj && cJSON_IsString(contentObj)) {
            snprintf(outText, outTextSize, "%s",
                     cJSON_GetStringValue(contentObj));
            success = true;
          } else {
            ESP_LOGE(TAG,
                     "Unexpected OpenAI response format (no message content)");
          }
        }
        cJSON_Delete(responseJson);
      }
    }
  }

  free(responseBuffer);
  cJSON_free(requestBody);
  esp_http_client_cleanup(client);
  return success;
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
  display.setTextWrap(true);
  display.setCursor(0, 0);
  display.print("Connecting to WiFi...");
  display.display();

  // Connect to WiFi using credentials configured via menuconfig
  if (display.wifi.begin() != ESP_OK || !display.wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi failed!");
    display.display();
    return;
  }

  ESP_LOGI(TAG, "WiFi connected");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Asking OpenAI...");
  display.display();

  char reply[OPENAI_REPLY_MAX_LEN];
  bool ok = requestOpenAI(PROMPT_TEXT, reply, sizeof(reply));

  display.clearDisplay();
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(20, 50);

  if (ok) {
    ESP_LOGI(TAG, "OpenAI reply: %s", reply);
    display.print(reply);
  } else {
    display.print("OpenAI request failed.");
  }

  display.display();
}
