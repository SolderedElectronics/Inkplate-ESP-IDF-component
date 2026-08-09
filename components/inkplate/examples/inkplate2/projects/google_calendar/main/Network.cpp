/**
 * @file        Network.cpp
 * @brief       Fetches upcoming events from the Google Calendar public API.
 */

#include "Network.h"

#include "calendarData.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

static const char *TAG = "CALENDAR_NET";

// Fallback buffer size used when the server doesn't send a Content-Length
// header (e.g. chunked responses). An events.list response for 14 events
// with descriptions can be several KB, so this is sized generously.
#define FALLBACK_RESPONSE_BUFFER_SIZE 16384
#define HTTP_TIMEOUT_MS 15000

NetworkFunctions::NetworkFunctions(const char *calendarID, const char *apiKey) {
  snprintf(this->calendarID, sizeof(this->calendarID), "%s", calendarID);
  snprintf(this->apiKey, sizeof(this->apiKey), "%s", apiKey);
}

bool NetworkFunctions::fetchCalendar(calendarData *data,
                                     int timezoneOffsetHours) {
  // time() always returns UTC seconds since the epoch regardless of any TZ
  // environment variable. The original sketch instead configured a "local"
  // NTP clock via configTime(timeZone * 3600, ...) and read it back with
  // getLocalTime(). The same effect is achieved here by adding the offset
  // manually to the UTC epoch (same technique used by this component's
  // clock/world_clock examples) - the caller (app_main()) must have already
  // synced the clock, e.g. via display.wifi.setCurrentTime(), before this
  // function is called.
  time_t nowLocal = time(nullptr) + (time_t)timezoneOffsetHours * 3600;
  struct tm timeInfo;
  gmtime_r(&nowLocal, &timeInfo);

  // Start of "today" (local), matching the original's timeMin. Note: like
  // the original sketch, the formatted string is suffixed with "Z" even
  // though the value has already been shifted by timezoneOffsetHours - this
  // quirk is carried over unchanged from the original for behavioral
  // parity, and is harmless here because the Calendar API only uses it as a
  // lower bound for a multi-day window.
  char timeMin[32];
  strftime(timeMin, sizeof(timeMin), "%Y-%m-%dT00:00:00Z", &timeInfo);

  // 14 days from now, end of day, matching the original's timeMax.
  time_t nowPlus14Days = nowLocal + 14 * 24 * 60 * 60;
  struct tm timeMaxInfo;
  gmtime_r(&nowPlus14Days, &timeMaxInfo);
  char timeMax[32];
  strftime(timeMax, sizeof(timeMax), "%Y-%m-%dT23:59:59Z", &timeMaxInfo);

  // Build the Google Calendar API URL (same query params as the original).
  char url[512];
  snprintf(url, sizeof(url),
           "https://www.googleapis.com/calendar/v3/calendars/%s/events"
           "?singleEvents=true&orderBy=startTime&timeMin=%s&timeMax=%s"
           "&maxResults=14&key=%s",
           calendarID, timeMin, timeMax, apiKey);

  esp_http_client_config_t config = {};
  config.url = url;
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = HTTP_TIMEOUT_MS;
  // googleapis.com is signed by a well-known public CA, so verify against
  // the ESP-IDF certificate bundle (same approach as the openai_text_prompt
  // example) rather than pinning a single certificate.
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
  int status = esp_http_client_get_status_code(client);
  if (status != 200) {
    ESP_LOGE(TAG, "HTTP error: %d (403 -> Calendar API not enabled, "
                  "404 -> calendar not public or wrong ID)",
             status);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  size_t bufSize =
      contentLen > 0 ? (size_t)contentLen : FALLBACK_RESPONSE_BUFFER_SIZE;
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

  cJSON *root = cJSON_Parse(buffer);
  free(buffer);
  if (!root) {
    ESP_LOGE(TAG, "JSON parse error");
    return false;
  }

  data->clearEvents();

  cJSON *items = cJSON_GetObjectItem(root, "items");
  if (cJSON_IsArray(items)) {
    cJSON *event = nullptr;
    cJSON_ArrayForEach(event, items) {
      cJSON *summaryObj = cJSON_GetObjectItem(event, "summary");
      const char *summary = (summaryObj && cJSON_IsString(summaryObj))
                                ? cJSON_GetStringValue(summaryObj)
                                : "No Title";

      cJSON *start = cJSON_GetObjectItem(event, "start");
      cJSON *end = cJSON_GetObjectItem(event, "end");

      // Timed events have "dateTime"; all-day events have "date" instead.
      cJSON *startDateTime =
          start ? cJSON_GetObjectItem(start, "dateTime") : nullptr;
      cJSON *startDate = start ? cJSON_GetObjectItem(start, "date") : nullptr;
      cJSON *endDateTime =
          end ? cJSON_GetObjectItem(end, "dateTime") : nullptr;
      cJSON *endDate = end ? cJSON_GetObjectItem(end, "date") : nullptr;

      const char *startStr =
          cJSON_IsString(startDateTime) ? cJSON_GetStringValue(startDateTime)
          : cJSON_IsString(startDate)   ? cJSON_GetStringValue(startDate)
                                        : "";
      const char *endStr =
          cJSON_IsString(endDateTime) ? cJSON_GetStringValue(endDateTime)
          : cJSON_IsString(endDate)   ? cJSON_GetStringValue(endDate)
                                      : "";

      data->addEvent(summary, startStr, endStr);
    }
  }

  cJSON_Delete(root);
  return true;
}
