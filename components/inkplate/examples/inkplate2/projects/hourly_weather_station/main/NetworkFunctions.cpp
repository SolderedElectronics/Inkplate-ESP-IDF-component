/**
 * NetworkFunctions.cpp
 * Inkplate ESP-IDF component - Inkplate 2 hourly weather station example
 *
 * Ported from the Inkplate-Arduino-library Inkplate2_Hourly_Weather_Station
 * example's Network.cpp. The original used HTTPClient + ArduinoJson; this
 * version uses the component's WiFi::downloadFileHTTPS() helper together
 * with cJSON (bundled with ESP-IDF).
 *
 * Original Arduino version:
 * David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek
 * https://github.com/e-radionicacom/Inkplate-6-Arduino-library
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 * This code is released under the GNU Lesser General Public License v3.0.
 */

// Uncomment for Fahrenheit / MPH output (matches AMERICAN define in main.cpp)
// #define AMERICAN

#include "NetworkFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "NETWORK";

// Response is usually a few KB; used only as a fallback when the server
// doesn't send a Content-Length header (see WiFi::downloadFileHTTPS()).
#define RESPONSE_BUFFER_FALLBACK_SIZE 65536

static void formatTemp(char *str, size_t len, double tempKelvin)
{
    double temp = tempKelvin - 273.15;
#ifdef AMERICAN
    temp = (temp * 9.0 / 5.0 + 32.0);
#endif
    snprintf(str, len, "%.0f", temp);
}

bool NetworkFunctions::getData(WiFi &wifi, const char *lon, const char *lat, const char *apiKey, char *temp1,
                                char *temp2, char *temp3, char *abbr1, char *abbr2, char *abbr3)
{
    // Build request URL
    char url[256];
    snprintf(url, sizeof(url),
             "https://api.openweathermap.org/data/3.0/onecall?lat=%s&lon=%s&appid=%s&units=metric", lat, lon,
             apiKey);

    int32_t len = RESPONSE_BUFFER_FALLBACK_SIZE;
    uint8_t *data = wifi.downloadFileHTTPS(url, &len);
    if (!data || len <= 0)
    {
        ESP_LOGE(TAG, "Failed to download weather data");
        return false;
    }

    cJSON *root = cJSON_ParseWithLength((const char *)data, (size_t)len);
    free(data);

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse weather JSON");
        return false;
    }

    bool ok = false;

    cJSON *current = cJSON_GetObjectItem(root, "current");
    cJSON *dt = current ? cJSON_GetObjectItem(current, "dt") : NULL;
    cJSON *tzOffset = cJSON_GetObjectItem(root, "timezone_offset");
    cJSON *hourly = cJSON_GetObjectItem(root, "hourly");

    if (cJSON_IsNumber(dt) && cJSON_IsNumber(tzOffset) && cJSON_IsArray(hourly) && cJSON_GetArraySize(hourly) >= 3)
    {
        dataEpoch = (time_t)cJSON_GetNumberValue(dt);
        timeZone = (int)(cJSON_GetNumberValue(tzOffset) / 3600.0);

        char *temps[3] = {temp1, temp2, temp3};
        char *abbrs[3] = {abbr1, abbr2, abbr3};
        ok = true;

        for (int i = 0; i < 3 && ok; ++i)
        {
            cJSON *hour = cJSON_GetArrayItem(hourly, i);
            cJSON *tempItem = hour ? cJSON_GetObjectItem(hour, "temp") : NULL;
            cJSON *weatherArr = hour ? cJSON_GetObjectItem(hour, "weather") : NULL;
            cJSON *weather0 = weatherArr ? cJSON_GetArrayItem(weatherArr, 0) : NULL;
            cJSON *icon = weather0 ? cJSON_GetObjectItem(weather0, "icon") : NULL;

            if (!cJSON_IsNumber(tempItem))
            {
                ok = false;
                break;
            }

            formatTemp(temps[i], 8, cJSON_GetNumberValue(tempItem));

            if (cJSON_IsString(icon))
                strlcpy(abbrs[i], cJSON_GetStringValue(icon), 16);
            else
                strlcpy(abbrs[i], "01d", 16);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected weather JSON structure");
    }

    cJSON_Delete(root);

    return ok;
}

void NetworkFunctions::getHours(char *hour1, char *hour2, char *hour3)
{
    long baseHour = dataEpoch / 3600L;

    snprintf(hour1, 8, "%2ldh", ((baseHour + timeZone) % 24 + 24) % 24);
    snprintf(hour2, 8, "%2ldh", ((baseHour + 1 + timeZone) % 24 + 24) % 24);
    snprintf(hour3, 8, "%2ldh", ((baseHour + 2 + timeZone) % 24 + 24) % 24);
}
