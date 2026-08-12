/**
 * @file WiFi.cpp
 * @author Fran Fodor for Soldered
 * @brief Helper for network communication.
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "string.h"
#include "time.h"

#include "WiFi.h"

static const char *TAG = "ESP_WIFI";

static bool resolveRedirectUrl(const char *baseUrl, const char *location,
                                char *out, size_t outSize) {
  if (strncmp(location, "http://", 7) == 0 ||
      strncmp(location, "https://", 8) == 0) {
    snprintf(out, outSize, "%s", location);
    return true;
  }

  const char *schemeEnd = strstr(baseUrl, "://");
  if (!schemeEnd)
    return false;

  size_t schemeLen = schemeEnd - baseUrl;
  const char *hostStart = schemeEnd + 3;
  const char *pathStart = strchr(hostStart, '/');
  size_t hostLen =
      pathStart ? (size_t)(pathStart - hostStart) : strlen(hostStart);

  snprintf(out, outSize, "%.*s://%.*s%s%s", (int)schemeLen, baseUrl,
           (int)hostLen, hostStart, location[0] == '/' ? "" : "/", location);
  return true;
}

static esp_err_t redirectLocationEventHandler(esp_http_client_event_t *evt) {
  if (evt->event_id == HTTP_EVENT_ON_HEADER && evt->user_data &&
      strcasecmp(evt->header_key, "Location") == 0) {
    snprintf((char *)evt->user_data, 512, "%s", evt->header_value);
  }
  return ESP_OK;
}

bool WiFi::m_connected = false;

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

esp_err_t WiFi::begin() {
  esp_err_t ret;

  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK)
    return ret;

  ret = esp_netif_init();
  if (ret != ESP_OK)
    return ret;

  ret = esp_event_loop_create_default();
  if (ret != ESP_OK)
    return ret;

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK)
    return ret;

  ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                            &wifiEventHandler, NULL, NULL);
  if (ret != ESP_OK)
    return ret;

  ret = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                            &ipEventHandler, NULL, NULL);
  if (ret != ESP_OK)
    return ret;

  if (strlen(CONFIG_WIFI_SSID) == 0) {
    ESP_LOGE(TAG, "WiFi SSID not set. Run menuconfig.");
    return ESP_ERR_INVALID_ARG;
  }

  wifi_config_t wifi_config = {};
  memcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(CONFIG_WIFI_SSID));
  memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD,
         sizeof(CONFIG_WIFI_PASSWORD));

  ret = esp_wifi_set_mode(WIFI_MODE_STA);
  if (ret != ESP_OK)
    return ret;

  ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (ret != ESP_OK)
    return ret;

  ret = esp_wifi_start();
  if (ret != ESP_OK)
    return ret;

  ESP_LOGI(TAG, "WiFi initialization finished!");
  return ESP_OK;
}

bool WiFi::waitForConnect(uint32_t timeoutMs) {
  uint32_t elapsed = 0;
  while (!m_connected && elapsed < timeoutMs) {
    vTaskDelay(pdMS_TO_TICKS(100));
    elapsed += 100;
  }
  return m_connected;
}

uint8_t *WiFi::downloadFile(const char *url, int32_t *len) {
  if (!waitForConnect()) {
    ESP_LOGE(TAG, "No WiFi connection");
    return NULL;
  }

  ESP_LOGI(TAG, "Downloading file via HTTP");

  char currentUrl[512];
  snprintf(currentUrl, sizeof(currentUrl), "%s", url);

  esp_http_client_handle_t client = NULL;
  int32_t contentLen = 0;
  int redirects = 0;

  while (true) {
    char locationBuf[512] = {0};

    esp_http_client_config_t config = {};
    config.url = currentUrl;
    config.timeout_ms = 10000;
    config.event_handler = redirectLocationEventHandler;
    config.user_data = locationBuf;
    if (strncmp(currentUrl, "https://", 8) == 0) {
      config.transport_type = HTTP_TRANSPORT_OVER_SSL;
      if (m_certificate)
        config.cert_pem = m_certificate;
    }

    client = esp_http_client_init(&config);
    if (!client)
      return NULL;

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
      esp_http_client_cleanup(client);
      return NULL;
    }

    contentLen = (int32_t)esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);

    if (status >= 300 && status < 400) {
      char nextUrl[512];
      if (locationBuf[0] == '\0' || redirects >= 5 ||
          !resolveRedirectUrl(currentUrl, locationBuf, nextUrl,
                               sizeof(nextUrl))) {
        ESP_LOGE(TAG, "Redirect failed (status %d)", status);
        esp_http_client_cleanup(client);
        return NULL;
      }
      snprintf(currentUrl, sizeof(currentUrl), "%s", nextUrl);
      esp_http_client_cleanup(client);
      redirects++;
      continue;
    }

    break;
  }

  int32_t capacity = contentLen > 0 ? contentLen : 4096;
  if (contentLen > 0)
    *len = contentLen;

  uint8_t *buffer = (uint8_t *)heap_caps_malloc(
      capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate %ld bytes", capacity);
    esp_http_client_cleanup(client);
    return NULL;
  }

  int32_t totalRead = 0;
  uint8_t chunk[512];
  int read;
  while (contentLen > 0 ? totalRead < contentLen : true) {
    read = esp_http_client_read(client, (char *)chunk, sizeof(chunk));
    if (read <= 0)
      break;
    if (totalRead + read > capacity) {
      int32_t newCapacity = capacity * 2;
      while (newCapacity < totalRead + read)
        newCapacity *= 2;
      uint8_t *newBuffer = (uint8_t *)heap_caps_realloc(
          buffer, newCapacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
      if (!newBuffer) {
        ESP_LOGE(TAG, "Failed to grow buffer to %ld bytes", newCapacity);
        heap_caps_free(buffer);
        esp_http_client_cleanup(client);
        return NULL;
      }
      buffer = newBuffer;
      capacity = newCapacity;
    }
    memcpy(buffer + totalRead, chunk, read);
    totalRead += read;
  }

  esp_http_client_cleanup(client);
  *len = totalRead;

  ESP_LOGI(TAG, "File downloaded");

  return buffer;
}

uint8_t *WiFi::downloadFileHTTPS(const char *url, int32_t *len) {
  if (!waitForConnect()) {
    ESP_LOGE(TAG, "No WiFi connection");
    return NULL;
  }

  ESP_LOGI(TAG, "Downloading file via HTTPS");

  char currentUrl[512];
  snprintf(currentUrl, sizeof(currentUrl), "%s", url);

  esp_http_client_handle_t client = NULL;
  int32_t contentLen = 0;
  int redirects = 0;

  while (true) {
    char locationBuf[512] = {0};

    esp_http_client_config_t config = {};
    config.url = currentUrl;
    config.timeout_ms = 10000;
    config.event_handler = redirectLocationEventHandler;
    config.user_data = locationBuf;
    if (strncmp(currentUrl, "https://", 8) == 0) {
      config.transport_type = HTTP_TRANSPORT_OVER_SSL;
      if (m_certificate)
        config.cert_pem = m_certificate;
    }

    client = esp_http_client_init(&config);
    if (!client)
      return NULL;

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "HTTPS open failed: %s", esp_err_to_name(err));
      esp_http_client_cleanup(client);
      return NULL;
    }

    contentLen = (int32_t)esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);

    if (status >= 300 && status < 400) {
      char nextUrl[512];
      if (locationBuf[0] == '\0' || redirects >= 5 ||
          !resolveRedirectUrl(currentUrl, locationBuf, nextUrl,
                               sizeof(nextUrl))) {
        ESP_LOGE(TAG, "Redirect failed (status %d)", status);
        esp_http_client_cleanup(client);
        return NULL;
      }
      snprintf(currentUrl, sizeof(currentUrl), "%s", nextUrl);
      esp_http_client_cleanup(client);
      redirects++;
      continue;
    }

    break;
  }

  int32_t capacity = contentLen > 0 ? contentLen : 4096;
  if (contentLen > 0)
    *len = contentLen;

  uint8_t *buffer = (uint8_t *)heap_caps_malloc(
      capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate %ld bytes", capacity);
    esp_http_client_cleanup(client);
    return NULL;
  }

  int32_t totalRead = 0;
  uint8_t chunk[512];
  int read;

  while (contentLen > 0 ? totalRead < contentLen : true) {
    read = esp_http_client_read(client, (char *)chunk, sizeof(chunk));
    if (read <= 0)
      break;
    if (totalRead + read > capacity) {
      int32_t newCapacity = capacity * 2;
      while (newCapacity < totalRead + read)
        newCapacity *= 2;
      uint8_t *newBuffer = (uint8_t *)heap_caps_realloc(
          buffer, newCapacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
      if (!newBuffer) {
        ESP_LOGE(TAG, "Failed to grow buffer to %ld bytes", newCapacity);
        heap_caps_free(buffer);
        esp_http_client_cleanup(client);
        return NULL;
      }
      buffer = newBuffer;
      capacity = newCapacity;
    }
    memcpy(buffer + totalRead, chunk, read);
    totalRead += read;
  }

  esp_http_client_cleanup(client);
  *len = totalRead;

  ESP_LOGI(TAG, "File downloaded");

  return buffer;
}

void WiFi::setCurrentTime() {
  esp_sntp_stop();

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_init();

  while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
    ESP_LOGI(TAG, "Waiting for NTP sync...");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

  setenv("TZ", "CST-2", 1);
  tzset();

  m_timeSet = true;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void WiFi::ipEventHandler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_id == IP_EVENT_STA_GOT_IP) {
    ESP_LOGI(TAG, "IP acquired");
    m_connected = true;
  }
  if (event_id == IP_EVENT_STA_LOST_IP) {
    ESP_LOGI(TAG, "IP lost");
    m_connected = false;
  }
}

void WiFi::wifiEventHandler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "Disconnected, retrying...");
    m_connected = false;
    esp_wifi_connect();
  }
}
