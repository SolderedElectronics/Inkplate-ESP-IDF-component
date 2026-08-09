/**
 * @file        Network.cpp
 * @brief       Fetches current/daily/hourly forecast data from the free
 *              Open-Meteo API (no API key required) and parses it with
 *              cJSON.
 *
 * @details     Ported from the Inkplate4TEMPERA_OpenMeteo_Weather_Station
 *              Arduino example's src/Network.cpp. See Network.h for a
 *              summary of what changed compared to the original
 *              HTTPClient + ArduinoJson version.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "Network.h"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

static const char *TAG = "OPENMETEO_NET";

// Fallback response-buffer size used if the server doesn't send a
// Content-Length header (e.g. chunked responses). The Open-Meteo response
// for this example's query set is typically a few KB.
#define FALLBACK_RESPONSE_BUFFER_SIZE (16 * 1024)
#define HTTP_TIMEOUT_MS 15000
// How many 1-second polls to wait for SNTP sync before giving up.
#define SNTP_SYNC_MAX_RETRIES 15

bool NetworkFunctions::setupTime(int utcOffsetHours, const char *ntpServer)
{
    esp_sntp_stop();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, ntpServer);
    esp_sntp_init();

    int retry = 0;
    while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < SNTP_SYNC_MAX_RETRIES)
    {
        ESP_LOGI(TAG, "Waiting for NTP sync... (%d/%d)", retry + 1, SNTP_SYNC_MAX_RETRIES);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }

    // POSIX TZ offsets are the negative of the common "UTC+N" convention:
    // "UTC-2" means local time = UTC - (-2) = UTC+2.
    char tz[16];
    snprintf(tz, sizeof(tz), "UTC%d", -utcOffsetHours);
    setenv("TZ", tz, 1);
    tzset();

    bool synced = retry < SNTP_SYNC_MAX_RETRIES;
    if (!synced)
        ESP_LOGW(TAG, "NTP sync timed out, continuing with whatever time is set");
    return synced;
}

const char *NetworkFunctions::getWeatherDescription(int code)
{
    switch (code)
    {
    case 0:
        return "Clear sky";
    case 1:
    case 2:
    case 3:
        return "Mainly clear, partly cloudy";
    case 45:
    case 48:
        return "Fog";
    case 51:
    case 53:
    case 55:
        return "Drizzle";
    case 56:
    case 57:
        return "Freezing Drizzle";
    case 61:
    case 63:
    case 65:
        return "Rain";
    case 66:
    case 67:
        return "Freezing Rain";
    case 71:
    case 73:
    case 75:
        return "Snowfall";
    case 77:
        return "Snow grains";
    case 80:
    case 81:
    case 82:
        return "Rain showers";
    case 85:
    case 86:
        return "Snow showers";
    case 95:
        return "Thunderstorm";
    case 96:
    case 99:
        return "Thunderstorm, hail";
    default:
        return "Unknown condition";
    }
}

void NetworkFunctions::extractDate(const char *dateTime, char *out, size_t outSize)
{
    const char *space = strchr(dateTime, ' ');
    if (space)
    {
        size_t len = (size_t)(space - dateTime);
        if (len >= outSize)
            len = outSize - 1;
        memcpy(out, dateTime, len);
        out[len] = '\0';
    }
    else
    {
        snprintf(out, outSize, "????-??-??");
    }
}

void NetworkFunctions::extractTime(const char *dateTime, char *out, size_t outSize)
{
    const char *space = strchr(dateTime, ' ');
    if (space && strlen(space + 1) >= 5)
        snprintf(out, outSize, "%.5s", space + 1); // e.g. "06:11"
    else
        snprintf(out, outSize, "??:??");
}

void NetworkFunctions::extractSun(const char *dateTime, char *out, size_t outSize)
{
    const char *t = strchr(dateTime, 'T');
    if (t && strlen(t + 1) >= 5)
        snprintf(out, outSize, "%.5s", t + 1); // e.g. "06:11"
    else
        snprintf(out, outSize, "??:??");
}

void NetworkFunctions::getFormattedTime(char *out, size_t outSize)
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    snprintf(out, outSize, "%04d-%02d-%02d %02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
             timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
}

int NetworkFunctions::getCurrentHour()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_hour;
}

const char *NetworkFunctions::getDayName(int dayIndex)
{
    static const char *daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    int dayOfWeekIndex = (timeinfo.tm_wday + dayIndex) % 7;
    return daysOfWeek[dayOfWeekIndex];
}

void NetworkFunctions::fetchWeatherData(WeatherData *weatherData, UserInfo *userInfo, float latitude,
                                        float longitude)
{
    char url[384];

    if (userInfo->useMetric)
    {
        snprintf(userInfo->temperatureLabel, sizeof(userInfo->temperatureLabel), " C");
        snprintf(userInfo->speedLabel, sizeof(userInfo->speedLabel), " km/h");
        snprintf(url, sizeof(url),
                 "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
                 "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,wind_speed_10m_max,"
                 "wind_direction_10m_dominant,precipitation_probability_max,weather_code,uv_index_max"
                 "&hourly=temperature_2m,precipitation_probability"
                 "&current=temperature_2m,precipitation,wind_speed_10m,weather_code,apparent_temperature,"
                 "relative_humidity_2m,is_day&timezone=auto",
                 (double)latitude, (double)longitude);
    }
    else
    {
        snprintf(userInfo->temperatureLabel, sizeof(userInfo->temperatureLabel), " F");
        snprintf(userInfo->speedLabel, sizeof(userInfo->speedLabel), " mph");
        snprintf(url, sizeof(url),
                 "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
                 "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,wind_speed_10m_max,"
                 "wind_direction_10m_dominant,precipitation_probability_max,weather_code,uv_index_max"
                 "&hourly=temperature_2m,precipitation_probability"
                 "&current=temperature_2m,precipitation,wind_speed_10m,weather_code,apparent_temperature,"
                 "relative_humidity_2m,is_day&timezone=auto"
                 "&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch",
                 (double)latitude, (double)longitude);
    }

    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = HTTP_TIMEOUT_MS;
    // open-meteo.com is signed by a well-known public CA, so verify it
    // against the ESP-IDF certificate bundle rather than disabling TLS
    // verification.
    config.crt_bundle_attach = esp_crt_bundle_attach;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        userInfo->apiError = true;
        return;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        userInfo->apiError = true;
        return;
    }

    int64_t contentLen = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    if (status != 200)
    {
        ESP_LOGE(TAG, "Unexpected HTTP status: %d", status);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        userInfo->apiError = true;
        return;
    }

    size_t bufSize = contentLen > 0 ? (size_t)contentLen : FALLBACK_RESPONSE_BUFFER_SIZE;
    char *buffer = (char *)malloc(bufSize + 1);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate %u bytes for response", (unsigned)(bufSize + 1));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        userInfo->apiError = true;
        return;
    }

    size_t totalRead = 0;
    int r;
    while (totalRead < bufSize && (r = esp_http_client_read(client, buffer + totalRead, bufSize - totalRead)) > 0)
    {
        totalRead += (size_t)r;
    }
    buffer[totalRead] = '\0';

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse Open-Meteo JSON response");
        userInfo->apiError = true;
        return;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    cJSON *daily = cJSON_GetObjectItem(root, "daily");
    cJSON *hourly = cJSON_GetObjectItem(root, "hourly");

    if (!current || !daily || !hourly)
    {
        ESP_LOGE(TAG, "Open-Meteo JSON response missing expected sections");
        cJSON_Delete(root);
        userInfo->apiError = true;
        return;
    }

    userInfo->apiError = false;

    // UserInfo timestamps (matches the original sketch's use of
    // getFormattedTime()/getCurrentHour() right after a successful fetch).
    getFormattedTime(userInfo->lastUpdated, sizeof(userInfo->lastUpdated));
    userInfo->currentHour = getCurrentHour();
    extractDate(userInfo->lastUpdated, userInfo->lastUpdatedDate, sizeof(userInfo->lastUpdatedDate));
    extractTime(userInfo->lastUpdated, userInfo->lastUpdatedTime, sizeof(userInfo->lastUpdatedTime));

    cJSON *item;

    item = cJSON_GetObjectItem(current, "temperature_2m");
    weatherData->currentTemp = item ? (float)item->valuedouble : 0.0f;
    item = cJSON_GetObjectItem(current, "apparent_temperature");
    weatherData->feelsLike = item ? (float)item->valuedouble : 0.0f;
    item = cJSON_GetObjectItem(current, "precipitation");
    weatherData->precipitation = item ? (float)item->valuedouble : 0.0f;
    item = cJSON_GetObjectItem(current, "wind_speed_10m");
    weatherData->windSpeed = item ? (float)item->valuedouble : 0.0f;
    item = cJSON_GetObjectItem(current, "is_day");
    weatherData->isDay = item && item->valueint != 0;
    item = cJSON_GetObjectItem(current, "weather_code");
    weatherData->weatherCode = item ? item->valueint : -1;
    snprintf(weatherData->weatherDescription, sizeof(weatherData->weatherDescription), "%s",
             getWeatherDescription(weatherData->weatherCode));

    cJSON *sunriseArr = cJSON_GetObjectItem(daily, "sunrise");
    cJSON *sunsetArr = cJSON_GetObjectItem(daily, "sunset");
    if (sunriseArr && cJSON_GetArraySize(sunriseArr) > 0)
        extractSun(cJSON_GetArrayItem(sunriseArr, 0)->valuestring, weatherData->sunrise,
                   sizeof(weatherData->sunrise));
    else
        snprintf(weatherData->sunrise, sizeof(weatherData->sunrise), "??:??");

    if (sunsetArr && cJSON_GetArraySize(sunsetArr) > 0)
        extractSun(cJSON_GetArrayItem(sunsetArr, 0)->valuestring, weatherData->sunset, sizeof(weatherData->sunset));
    else
        snprintf(weatherData->sunset, sizeof(weatherData->sunset), "??:??");

    cJSON *uvArr = cJSON_GetObjectItem(daily, "uv_index_max");
    weatherData->uvIndex =
        (uvArr && cJSON_GetArraySize(uvArr) > 0) ? (float)cJSON_GetArrayItem(uvArr, 0)->valuedouble : 0.0f;

    // Fetch daily forecast for the next 7 days.
    cJSON *minArr = cJSON_GetObjectItem(daily, "temperature_2m_min");
    cJSON *maxArr = cJSON_GetObjectItem(daily, "temperature_2m_max");
    cJSON *codeArr = cJSON_GetObjectItem(daily, "weather_code");

    for (int i = 0; i < 7; i++)
    {
        weatherData->dailyMinTemp[i] =
            (minArr && i < cJSON_GetArraySize(minArr)) ? (float)cJSON_GetArrayItem(minArr, i)->valuedouble : 0.0f;
        weatherData->dailyMaxTemp[i] =
            (maxArr && i < cJSON_GetArraySize(maxArr)) ? (float)cJSON_GetArrayItem(maxArr, i)->valuedouble : 0.0f;
        weatherData->dailyWeatherCodes[i] =
            (codeArr && i < cJSON_GetArraySize(codeArr)) ? cJSON_GetArrayItem(codeArr, i)->valueint : 0;
        snprintf(weatherData->dailyNames[i], sizeof(weatherData->dailyNames[i]), "%s", getDayName(i));
    }

    // Fetch data for the hourly temperature/precipitation graph, starting
    // at the current hour. Unlike the original sketch, the source index is
    // clamped to the array bounds instead of indexing past the end of the
    // array near the end of the day.
    cJSON *hourlyTempArr = cJSON_GetObjectItem(hourly, "temperature_2m");
    cJSON *hourlyPrecipArr = cJSON_GetObjectItem(hourly, "precipitation_probability");
    cJSON *hourlyTimeArr = cJSON_GetObjectItem(hourly, "time");
    int hourlyCount = hourlyTimeArr ? cJSON_GetArraySize(hourlyTimeArr) : 0;

    for (int i = 0; i < 6; i++)
    {
        int sourceIndex = userInfo->currentHour + i;
        if (hourlyCount > 0 && sourceIndex >= hourlyCount)
            sourceIndex = hourlyCount - 1;

        weatherData->hourlyTemps[i] = (hourlyTempArr && sourceIndex < cJSON_GetArraySize(hourlyTempArr))
                                          ? (float)cJSON_GetArrayItem(hourlyTempArr, sourceIndex)->valuedouble
                                          : 0.0f;
        weatherData->hourlyPrecip[i] = (hourlyPrecipArr && sourceIndex < cJSON_GetArraySize(hourlyPrecipArr))
                                            ? (float)cJSON_GetArrayItem(hourlyPrecipArr, sourceIndex)->valuedouble
                                            : 0.0f;
        if (hourlyTimeArr && sourceIndex < cJSON_GetArraySize(hourlyTimeArr))
            extractSun(cJSON_GetArrayItem(hourlyTimeArr, sourceIndex)->valuestring, weatherData->hourlyTimes[i],
                       sizeof(weatherData->hourlyTimes[i]));
        else
            snprintf(weatherData->hourlyTimes[i], sizeof(weatherData->hourlyTimes[i]), "??:??");
    }

    cJSON_Delete(root);
}
