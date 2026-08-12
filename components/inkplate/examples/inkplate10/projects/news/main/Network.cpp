/**
 * Network.cpp
 * Inkplate ESP-IDF component - Inkplate 10 news example
 *
 * Ported from the Inkplate-Arduino-library Inkplate10_News example's
 * src/Network.cpp. The original used HTTPClient + ArduinoJson and manual
 * WiFi (re)connection handling; this version uses esp_http_client directly
 * (so the ESP-IDF certificate bundle can be attached for TLS verification)
 * together with cJSON. WiFi connect/reconnect is handled by the shared
 * WiFi helper (display.wifi), which auto-reconnects on disconnect, so the
 * manual reconnect-and-restart loop from the original is no longer needed.
 * This mirrors the technique already used by this component's Inkplate 5
 * news example.
 *
 * Original Arduino version:
 * Matej Andracic @ Soldered
 * https://github.com/SolderedElectronics/Inkplate-Arduino-library/tree/master/examples/Inkplate10
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 * This code is released under the GNU Lesser General Public License v3.0.
 */

#include "Network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "NETWORK";

// NewsAPI.org's default page size is 20 articles; cap the parsed array at
// that so memory usage stays bounded regardless of the response.
#define MAX_ARTICLES 20

#define HTTP_TIMEOUT_MS 15000

// The JSON response is usually a few KB to a few tens of KB, depending on
// how many/how long the article descriptions are.
#define RESPONSE_BUFFER_SIZE (48 * 1024)

void NetworkFunctions::setApiKey(const char *apiKey)
{
    strncpy(apiKeyNews, apiKey, sizeof(apiKeyNews) - 1);
}

news *NetworkFunctions::getData(Inkplate &display, int *outCount)
{
    *outCount = 0;

    // Make sure WiFi is connected before attempting the request. The WiFi
    // helper reconnects automatically on disconnect events, so a short wait
    // is normally enough if a reconnect is already in progress.
    if (!display.wifi.isConnected() && !display.wifi.waitForConnect())
    {
        ESP_LOGE(TAG, "No WiFi connection");
        return nullptr;
    }

    // Build the request URL (API key passed as a query parameter, same as
    // the original Arduino sketch).
    char url[192];
    snprintf(url, sizeof(url), "https://newsapi.org/v2/top-headlines?country=us&apiKey=%s", apiKeyNews);

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = HTTP_TIMEOUT_MS;
    // newsapi.org is signed by a well-known public CA, so verify it against
    // the ESP-IDF certificate bundle rather than disabling TLS verification.
    config.crt_bundle_attach = esp_crt_bundle_attach;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return nullptr;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return nullptr;
    }

    int64_t contentLength = esp_http_client_fetch_headers(client);
    int statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "NewsAPI HTTP status=%d content-length=%lld", statusCode, (long long)contentLength);

    char *buffer = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate response buffer");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return nullptr;
    }

    int totalRead = 0;
    int r;
    while (totalRead < RESPONSE_BUFFER_SIZE - 1 &&
           (r = esp_http_client_read(client, buffer + totalRead, RESPONSE_BUFFER_SIZE - 1 - totalRead)) > 0)
    {
        totalRead += r;
    }
    buffer[totalRead] = '\0';

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (statusCode == 404)
    {
        // Handle case where no news is found (matches the original sketch's
        // on-screen message, minus the infinite halt - the caller's refresh
        // loop will simply try again next cycle).
        display.clearDisplay();
        display.setCursor(50, 230);
        display.setTextSize(2);
        display.println("No news");
        display.display();
        free(buffer);
        return nullptr;
    }

    if (statusCode != 200)
    {
        ESP_LOGE(TAG, "NewsAPI request failed (status %d): %s", statusCode, buffer);
        free(buffer);
        return nullptr;
    }

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse NewsAPI JSON");
        return nullptr;
    }

    news *ent = nullptr;

    cJSON *status = cJSON_GetObjectItem(root, "status");
    cJSON *articles = cJSON_GetObjectItem(root, "articles");

    if (cJSON_IsString(status) && strcmp(cJSON_GetStringValue(status), "ok") == 0 && cJSON_IsArray(articles))
    {
        int n = cJSON_GetArraySize(articles);
        if (n > MAX_ARTICLES)
            n = MAX_ARTICLES;

        ESP_LOGI(TAG, "Number of articles: %d", n);

        // `new news[n]` default-constructs every element (title/description
        // = nullptr), unlike the original's ps_malloc() which left the
        // memory uninitialized until explicitly set below.
        ent = new news[n];

        for (int i = 0; i < n; i++)
        {
            cJSON *article = cJSON_GetArrayItem(articles, i);
            cJSON *title = article ? cJSON_GetObjectItem(article, "title") : nullptr;
            cJSON *description = article ? cJSON_GetObjectItem(article, "description") : nullptr;

            if (cJSON_IsString(title))
                ent[i].title = strdup(cJSON_GetStringValue(title));
            if (cJSON_IsString(description))
                ent[i].description = strdup(cJSON_GetStringValue(description));
        }

        *outCount = n;
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected NewsAPI JSON structure");
    }

    cJSON_Delete(root);

    return ent;
}
