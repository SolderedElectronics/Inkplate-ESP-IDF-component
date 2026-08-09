/**
 * @file        Network.cpp
 * @brief       Fetches events from a public Google Calendar via the Google
 *              Calendar API (HTTP GET + cJSON, using esp_http_client).
 */
#include "Network.h"
#include "includes.h" // getLocalTimeAdjusted()

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char *TAG = "CALENDAR_NET";

#define CALENDAR_API_HOST "https://www.googleapis.com/calendar/v3/calendars/"
#define CALENDAR_URL_MAX_LEN 512
// Fallback buffer size used when the server doesn't send a Content-Length
// header (e.g. chunked responses).
#define CALENDAR_RESPONSE_FALLBACK_SIZE 8192
#define TIME_SYNC_RETRY_ATTEMPTS 10
#define TIME_SYNC_RETRY_DELAY_MS 1000
// How many days ahead of "today" to request events for.
#define CALENDAR_LOOKAHEAD_DAYS 14
// Matches the original sketch's maxResults request parameter.
#define CALENDAR_MAX_RESULTS 14

NetworkFunctions::NetworkFunctions(const char *calendarID, const char *apiKey) {
  snprintf(this->calendarID, sizeof(this->calendarID), "%s", calendarID);
  snprintf(this->apiKey, sizeof(this->apiKey), "%s", apiKey);
}

bool NetworkFunctions::fetchCalendar(calendarData *data) {
  // Retry loop to wait for the SNTP time sync (up to ~10 seconds), matching
  // the original sketch's wait-for-getLocalTime() loop.
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTimeAdjusted(&timeinfo) && attempts < TIME_SYNC_RETRY_ATTEMPTS) {
    ESP_LOGI(TAG, "Waiting for time sync...");
    vTaskDelay(pdMS_TO_TICKS(TIME_SYNC_RETRY_DELAY_MS));
    attempts++;
  }
  if (!getLocalTimeAdjusted(&timeinfo)) {
    ESP_LOGE(TAG, "Cannot fetch calendar - time not available");
    return false;
  }

  // Set start time to today's date at 00:00:00, and end time
  // CALENDAR_LOOKAHEAD_DAYS days from now at 23:59:59. Both are computed
  // directly from the (already timezone-shifted) epoch returned by
  // getLocalTimeAdjusted() rather than via mktime(), so the result does not
  // depend on the C library's TZ environment variable.
  time_t shiftedNow = time(nullptr) + (time_t)TIME_ZONE_OFFSET_HOURS * 3600;

  char timeMin[32];
  strftime(timeMin, sizeof(timeMin), "%Y-%m-%dT00:00:00Z", &timeinfo);

  time_t shiftedMax = shiftedNow + (time_t)CALENDAR_LOOKAHEAD_DAYS * 24 * 60 * 60;
  struct tm timeMaxInfo;
  gmtime_r(&shiftedMax, &timeMaxInfo);

  char timeMax[32];
  strftime(timeMax, sizeof(timeMax), "%Y-%m-%dT23:59:59Z", &timeMaxInfo);

  // Build the Google Calendar API "list events" URL.
  char url[CALENDAR_URL_MAX_LEN];
  snprintf(url, sizeof(url),
           "%s%s/events?singleEvents=true&orderBy=startTime&timeMin=%s&"
           "timeMax=%s&maxResults=%d&key=%s",
           CALENDAR_API_HOST, calendarID, timeMin, timeMax,
           CALENDAR_MAX_RESULTS, apiKey);

  // googleapis.com is signed by a well-known public CA, so verify the server
  // certificate using the ESP-IDF certificate bundle rather than pinning a
  // single certificate.
  esp_http_client_config_t config = {};
  config.url = url;
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = 15000;
  config.crt_bundle_attach = esp_crt_bundle_attach;

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

  int64_t contentLen = esp_http_client_fetch_headers(client);
  int statusCode = esp_http_client_get_status_code(client);
  if (statusCode != 200) {
    ESP_LOGE(TAG, "HTTP error: %d", statusCode);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  size_t bufSize = contentLen > 0 ? (size_t)contentLen
                                  : CALENDAR_RESPONSE_FALLBACK_SIZE;
  char *buffer = (char *)malloc(bufSize + 1);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for response",
             (unsigned)(bufSize + 1));
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  size_t totalRead = 0;
  int r;
  while (totalRead < bufSize &&
         (r = esp_http_client_read(client, buffer + totalRead,
                                   bufSize - totalRead)) > 0) {
    totalRead += (size_t)r;
  }
  buffer[totalRead] = '\0';

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  // Parse the JSON response.
  cJSON *root = cJSON_Parse(buffer);
  free(buffer);
  if (!root) {
    ESP_LOGE(TAG, "JSON parse error");
    return false;
  }

  // Clear existing events and add new ones.
  data->clearEvents();

  cJSON *items = cJSON_GetObjectItem(root, "items");
  if (cJSON_IsArray(items)) {
    cJSON *event;
    cJSON_ArrayForEach(event, items) {
      cJSON *summaryObj = cJSON_GetObjectItem(event, "summary");
      const char *summary = (summaryObj && cJSON_IsString(summaryObj))
                                ? cJSON_GetStringValue(summaryObj)
                                : "No Title";

      cJSON *start = cJSON_GetObjectItem(event, "start");
      cJSON *end = cJSON_GetObjectItem(event, "end");

      cJSON *startDateTime = start ? cJSON_GetObjectItem(start, "dateTime") : nullptr;
      cJSON *startDate = start ? cJSON_GetObjectItem(start, "date") : nullptr;
      cJSON *endDateTime = end ? cJSON_GetObjectItem(end, "dateTime") : nullptr;
      cJSON *endDate = end ? cJSON_GetObjectItem(end, "date") : nullptr;

      const char *startStr = cJSON_IsString(startDateTime)
                                 ? cJSON_GetStringValue(startDateTime)
                                 : (cJSON_IsString(startDate)
                                        ? cJSON_GetStringValue(startDate)
                                        : "");
      const char *endStr = cJSON_IsString(endDateTime)
                               ? cJSON_GetStringValue(endDateTime)
                               : (cJSON_IsString(endDate)
                                      ? cJSON_GetStringValue(endDate)
                                      : "");

      data->addEvent(summary, startStr, endStr);
    }
  }

  cJSON_Delete(root);
  return true;
}
