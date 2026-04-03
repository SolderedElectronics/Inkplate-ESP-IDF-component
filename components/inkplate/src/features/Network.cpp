#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "time.h"
#include "string.h"

#include "Network.h"

static const char *TAG = "ESP_WIFI";

bool WiFi::m_connected = false;

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief Initialize WiFi in STA mode with credentials from menuconfig.
 *
 */
WiFi::WiFi()
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ipEventHandler, NULL, NULL));

  if (strlen(CONFIG_WIFI_SSID) == 0) {
  ESP_LOGE(TAG, "WiFi SSID not set. Run menuconfig.");
  return;
  }

  wifi_config_t wifi_config = {};
  memcpy(wifi_config.sta.ssid,     CONFIG_WIFI_SSID,     sizeof(CONFIG_WIFI_SSID));
  memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(CONFIG_WIFI_PASSWORD));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WiFi initialization finished!");
}

/**
 * @brief Block until an IP is acquired or the timeout elapses.
 *
 * @param  uint32_t timeoutMs
 *         Maximum time to wait in milliseconds.
 *
 * @return true if connected, false if timed out.
 */
bool WiFi::waitForConnect(uint32_t timeoutMs)
{
  uint32_t elapsed = 0;
  while (!m_connected && elapsed < timeoutMs)
  {
  vTaskDelay(pdMS_TO_TICKS(100));
  elapsed += 100;
  }
  return m_connected;
}

/**
 * @brief Download a file via HTTP.
 *
 * @param  const char *url
 *         URL to download.
 *
 * @param  int32_t *len
 *         In: assumed length if server doesn't provide Content-Length.
 *         Out: actual number of bytes downloaded.
 *
 * @return uint8_t* pointer to allocated buffer, or NULL on failure.
 *         Caller is responsible for freeing the buffer.
 */
uint8_t *WiFi::downloadFile(const char *url, int32_t *len)
{
  if (!waitForConnect())
  {
  ESP_LOGE(TAG, "No WiFi connection");
  return NULL;
  }

  esp_http_client_config_t config = {};
  config.url         = url;
  config.timeout_ms  = 10000;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client)
  return NULL;

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK)
  {
  ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
  esp_http_client_cleanup(client);
  return NULL;
  }

  int32_t contentLen = (int32_t)esp_http_client_fetch_headers(client);
  if (contentLen <= 0)
  contentLen = *len;
  else
  *len = contentLen;

  uint8_t *buffer = (uint8_t *)heap_caps_malloc(contentLen, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buffer)
  {
  ESP_LOGE(TAG, "Failed to allocate %ld bytes", contentLen);
  esp_http_client_cleanup(client);
  return NULL;
  }

  int32_t totalRead = 0;
  uint8_t chunk[512];
  int read;
  while (totalRead < contentLen)
  {
  read = esp_http_client_read(client, (char *)chunk, sizeof(chunk));
  if (read <= 0)
    break;
  memcpy(buffer + totalRead, chunk, read);
  totalRead += read;
  }

  esp_http_client_cleanup(client);
  *len = totalRead;

  return buffer;
}

/**
 * @brief  Download a file via HTTPS.
 *
 * @param  const char *url
 *         URL to download.
 *
 * @param  int32_t *len
 *         In: assumed length if server doesn't provide Content-Length.
 *         Out: actual number of bytes downloaded.
 *
 * @return uint8_t* pointer to allocated buffer, or NULL on failure.
 *         Caller is responsible for freeing the buffer.
 */
uint8_t *WiFi::downloadFileHTTPS(const char *url, int32_t *len)
{
  if (!waitForConnect())
  {
  ESP_LOGE(TAG, "No WiFi connection");
  return NULL;
  }

  esp_http_client_config_t config = {};
  config.url            = url;
  config.timeout_ms     = 10000;
  config.transport_type = HTTP_TRANSPORT_OVER_SSL;

  if (m_certificate)
  config.cert_pem = m_certificate;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client)
  return NULL;

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK)
  {
  ESP_LOGE(TAG, "HTTPS open failed: %s", esp_err_to_name(err));
  esp_http_client_cleanup(client);
  return NULL;
  }

  int32_t contentLen = (int32_t)esp_http_client_fetch_headers(client);
  if (contentLen <= 0)
  contentLen = *len;
  else
  *len = contentLen;

  uint8_t *buffer = (uint8_t *)heap_caps_malloc(contentLen, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buffer)
  {
  ESP_LOGE(TAG, "Failed to allocate %ld bytes", contentLen);
  esp_http_client_cleanup(client);
  return NULL;
  }

  int32_t totalRead = 0;
  uint8_t chunk[512];
  int     read;
  while (totalRead < contentLen)
  {
  read = esp_http_client_read(client, (char *)chunk, sizeof(chunk));
  if (read <= 0)
    break;
  memcpy(buffer + totalRead, chunk, read);
  totalRead += read;
  }

  esp_http_client_cleanup(client);
  *len = totalRead;

  return buffer;
}

/**
 * @brief Set current time in CET timezone using SNTP.
 *
 */
void WiFi::setCurrentTime()
{
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
/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief Handle IP events.
 */
void WiFi::ipEventHandler(void *arg, esp_event_base_t event_base,
              int32_t event_id, void *event_data)
{
  if (event_id == IP_EVENT_STA_GOT_IP)
  {
  ESP_LOGI(TAG, "IP acquired");
  m_connected = true;
  }
  if (event_id == IP_EVENT_STA_LOST_IP)
  {
  ESP_LOGI(TAG, "IP lost");
  m_connected = false;
  }
}

/**
 * @brief Handle WiFi events such as starting and disconnecting.
 */
void WiFi::wifiEventHandler(void *arg, esp_event_base_t event_base,
              int32_t event_id, void *event_data)
{
  if (event_id == WIFI_EVENT_STA_START)
  {
  esp_wifi_connect();
  }
  else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
  ESP_LOGI(TAG, "Disconnected, retrying...");
  m_connected = false;
  esp_wifi_connect();
  }
}
