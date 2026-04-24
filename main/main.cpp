#include "Inkplate.h"
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "HTTPS_POST";

#define DELAY_BETWEEN_REQUESTS_MS 10000
#define API_URL "https://jsonplaceholder.typicode.com/posts"

// Response buffer
static char s_responseBuf[4096];
static int  s_responseLen = 0;

static esp_err_t httpEventHandler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (s_responseLen + evt->data_len < (int)sizeof(s_responseBuf) - 1)
        {
            memcpy(s_responseBuf + s_responseLen, evt->data, evt->data_len);
            s_responseLen += evt->data_len;
            s_responseBuf[s_responseLen] = '\0';
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        s_responseLen = 0; // reset for next request
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void sendPost()
{
    // Build JSON payload manually — no ArduinoJson in ESP-IDF
    char jsonBody[128];
    snprintf(jsonBody, sizeof(jsonBody), "{\"title\":\"Hello Inkplate\"}");

    s_responseLen = 0;
    memset(s_responseBuf, 0, sizeof(s_responseBuf));

    esp_http_client_config_t config = {};
    config.url             = API_URL;
    config.method          = HTTP_METHOD_POST;
    config.timeout_ms      = 10000;
    config.event_handler   = httpEventHandler;
    // Skip certificate validation — same as client.setInsecure() in Arduino
    config.skip_cert_common_name_check = true;
    config.transport_type  = HTTP_TRANSPORT_OVER_SSL;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, jsonBody, strlen(jsonBody));

    esp_err_t ret = esp_http_client_perform(client);
    if (ret == ESP_OK)
    {
        int statusCode = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Status code: %d", statusCode);
        ESP_LOGI(TAG, "Response: %s", s_responseBuf);
    }
    else
    {
        ESP_LOGE(TAG, "HTTPS POST failed: %s", esp_err_to_name(ret));
    }

    esp_http_client_cleanup(client);
}

extern "C" void app_main(void)
{
    static Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);
    display.setTextColor(BLACK, WHITE);
    display.setTextWrap(true);

    display.setTextSize(5);
    display.setCursor(0, 0);
    display.print("HTTPS POST\nRequest example\n");
    display.setTextSize(3);
    display.print("\nCheck ESP-IDF logs\nfor response");
    display.display();

    WiFi wifi;
    if (wifi.begin() != ESP_OK || !wifi.waitForConnect(50000))
    {
        ESP_LOGE(TAG, "WiFi connection failed");
        return;
    }
    ESP_LOGI(TAG, "WiFi connected");

    while (true)
    {
        sendPost();
        vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_REQUESTS_MS));
    }
}