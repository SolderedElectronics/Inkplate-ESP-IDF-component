/**
 * @file        QuotablesNetwork.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Helper for fetching a random quote from the Quotable public API.
 *
 * @details     Ported from the Inkplate10_Quotables Arduino example. WiFi
 *              connection handling is left to app_main() (via display.wifi),
 *              matching the rest of this component's examples; this file is
 *              only responsible for the HTTP GET + JSON parsing step that the
 *              original NetworkFunctions::getData() performed with
 *              HTTPClient + ArduinoJson.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "QuotablesNetwork.h"

#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <cstdlib>
#include <cstring>

static const char *TAG = "QUOTABLE_NET";

// Public API endpoint that returns a random quote as JSON. No API key
// required.
#define QUOTE_API_URL "https://api.quotable.kurokeita.dev/api/quotes/random"

// Fallback buffer size used when the server doesn't send a Content-Length
// header (e.g. chunked responses).
#define FALLBACK_RESPONSE_BUFFER_SIZE 4096

bool NetworkFunctions::getData(char *text, size_t textSize, char *author,
                               size_t authorSize) {
  esp_http_client_config_t config = {};
  config.url = QUOTE_API_URL;
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = 10000;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    return false;
  }

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return false;
  }

  // fetch_headers() also gives us the Content-Length (or -1 if unknown).
  int64_t contentLen = esp_http_client_fetch_headers(client);
  int status = esp_http_client_get_status_code(client);
  if (status != 200) {
    ESP_LOGE(TAG, "Unexpected HTTP status: %d", status);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  size_t bufSize = contentLen > 0 ? (size_t)contentLen
                                  : FALLBACK_RESPONSE_BUFFER_SIZE;
  char *buffer = (char *)malloc(bufSize + 1);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for response",
             (unsigned)(bufSize + 1));
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  size_t totalRead = 0;
  int read;
  while (totalRead < bufSize &&
         (read = esp_http_client_read(client, buffer + totalRead,
                                      bufSize - totalRead)) > 0) {
    totalRead += (size_t)read;
  }
  buffer[totalRead] = '\0';

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  // Some APIs prepend extra whitespace before the JSON body; skip ahead to
  // the first '{' just like the original Arduino sketch did on the raw
  // stream.
  char *jsonStart = buffer;
  while (*jsonStart != '\0' && *jsonStart != '{')
    jsonStart++;

  cJSON *root = cJSON_Parse(jsonStart);
  free(buffer);

  if (!root) {
    ESP_LOGE(TAG, "Failed to parse JSON response");
    return false;
  }

  bool ok = false;
  cJSON *quote = cJSON_GetObjectItem(root, "quote");
  if (quote) {
    cJSON *content = cJSON_GetObjectItem(quote, "content");
    cJSON *authorObj = cJSON_GetObjectItem(quote, "author");
    cJSON *name = authorObj ? cJSON_GetObjectItem(authorObj, "name") : NULL;

    const char *contentStr = cJSON_GetStringValue(content);
    const char *nameStr = cJSON_GetStringValue(name);

    if (contentStr && nameStr) {
      snprintf(text, textSize, "%s", contentStr);
      snprintf(author, authorSize, "%s", nameStr);
      ok = true;
    } else {
      ESP_LOGE(TAG, "Quote JSON missing 'content' or 'author.name' field");
    }
  } else {
    ESP_LOGE(TAG, "Quote JSON missing 'quote' object");
  }

  cJSON_Delete(root);
  return ok;
}
