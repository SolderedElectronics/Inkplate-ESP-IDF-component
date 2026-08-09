/**
 * @file        Network.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Fetches current conditions + forecast from the Open-Meteo API.
 *
 * @details     Ported from the Inkplate2_OpenMeteo_Weather_Station Arduino
 *              example's src/Network.cpp. WiFi connection handling is left
 *              to app_main() (via display.wifi), matching the rest of this
 *              component's examples; this file is only responsible for the
 *              HTTP GET + JSON parsing step that the original performed with
 *              HTTPClient + ArduinoJson.
 *
 *              The original derived "now" (used to pick the hourly-forecast
 *              start index and to name the 7 forecast days) from configTime()
 *              + getLocalTime(), i.e. the ESP32's NTP-synced clock in a
 *              user-supplied UTC offset. Open-Meteo is requested with
 *              "timezone=auto", so it already returns every timestamp
 *              (current.time, daily.time, hourly.time, sunrise/sunset) in the
 *              requested location's *local* time - this port reads "now"
 *              straight out of current.time/daily.time[0] instead, which
 *              removes the NTP/timezone-offset dependency entirely and is
 *              correct even when the device's own clock (fixed to CET by
 *              WiFi::setCurrentTime(), see components/inkplate/features/WiFi.h)
 *              doesn't match the requested location's timezone.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "Network.h"

#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char *TAG = "OPENMETEO_NET";

// Fallback buffer size used when the server doesn't send a Content-Length
// header (e.g. chunked responses).
#define FALLBACK_RESPONSE_BUFFER_SIZE 8192

// Function to get weather description based on the WMO weather code
const char *NetworkFunctions::getWeatherDescription(int code)
{
    switch (code)
    {
    case 0:
        return "Clear sky";
    case 1:
    case 2:
    case 3:
        return "Partly cloudy";
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
    case 96:
    case 99:
        return "Thunderstorm";
    default:
        return "Unknown condition";
    }
}

// Splits an ISO-8601 "YYYY-MM-DDTHH:MM" timestamp into its date portion.
void NetworkFunctions::extractDate(const char *dateTime, char *out, size_t outSize)
{
    const char *tPos = dateTime ? strchr(dateTime, 'T') : NULL;
    if (tPos)
    {
        size_t len = (size_t)(tPos - dateTime);
        if (len >= outSize)
            len = outSize - 1;
        memcpy(out, dateTime, len);
        out[len] = '\0';
    }
    else
    {
        snprintf(out, outSize, "?\?\?\?-\?\?-\?\?");
    }
}

// Splits an ISO-8601 "YYYY-MM-DDTHH:MM" timestamp into its "HH:MM" portion.
void NetworkFunctions::extractTime(const char *dateTime, char *out, size_t outSize)
{
    const char *tPos = dateTime ? strchr(dateTime, 'T') : NULL;
    if (tPos && strlen(tPos + 1) >= 5)
    {
        snprintf(out, outSize, "%.5s", tPos + 1);
    }
    else
    {
        snprintf(out, outSize, "??:??");
    }
}

// Same as extractTime() - the original Network.cpp had a separate
// extractSun() because sunrise/sunset used a different timestamp format than
// the self-built lastUpdated timestamp; since every timestamp in this port
// comes straight from Open-Meteo's ISO-8601 fields, both now do the same
// thing. Kept as a separate method to mirror the original file's structure.
void NetworkFunctions::extractSun(const char *dateTime, char *out, size_t outSize)
{
    extractTime(dateTime, out, outSize);
}

// Returns the day-of-week name for "weekday0 + dayIndex" (0 = Sunday).
const char *NetworkFunctions::getDayName(int weekday0, int dayIndex)
{
    static const char *daysOfWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    int idx = ((weekday0 + dayIndex) % 7 + 7) % 7;
    return daysOfWeek[idx];
}

// Computes the day of the week (0 = Sunday) for a "YYYY-MM-DD" date using
// Sakamoto's algorithm, so forecast day names can be derived directly from
// daily.time[0] without needing the device's own clock.
static int dayOfWeekFromDate(const char *dateStr)
{
    int y, m, d;
    if (!dateStr || sscanf(dateStr, "%d-%d-%d", &y, &m, &d) != 3)
        return 0;

    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3)
        y -= 1;
    int dow = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
    return dow < 0 ? dow + 7 : dow;
}

// Function to fetch weather data from the Open-Meteo API
void NetworkFunctions::fetchWeatherData(WeatherData *weatherData, UserInfo *userInfo, float latitude, float longitude)
{
    // Build the request URL. Only the daily/hourly/current fields actually
    // read by WeatherData below are requested (the original sketch also
    // requested wind_speed_10m_max, wind_direction_10m_dominant and
    // precipitation_probability_max, but never stored or displayed them).
    char url[512];
    if (userInfo->useMetric)
    {
        snprintf(userInfo->temperatureLabel, sizeof(userInfo->temperatureLabel), " C");
        snprintf(userInfo->speedLabel, sizeof(userInfo->speedLabel), " km/h");
        snprintf(url, sizeof(url),
                 "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
                 "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,weather_code,uv_index_max"
                 "&hourly=temperature_2m,precipitation_probability"
                 "&current=temperature_2m,precipitation,wind_speed_10m,weather_code,apparent_temperature,relative_"
                 "humidity_2m,is_day"
                 "&timezone=auto",
                 latitude, longitude);
    }
    else
    {
        snprintf(userInfo->temperatureLabel, sizeof(userInfo->temperatureLabel), " F");
        snprintf(userInfo->speedLabel, sizeof(userInfo->speedLabel), " mph");
        snprintf(url, sizeof(url),
                 "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
                 "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,weather_code,uv_index_max"
                 "&hourly=temperature_2m,precipitation_probability"
                 "&current=temperature_2m,precipitation,wind_speed_10m,weather_code,apparent_temperature,relative_"
                 "humidity_2m,is_day"
                 "&timezone=auto&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch",
                 latitude, longitude);
    }

    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = 10000;

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

    // fetch_headers() also gives us the Content-Length (or -1 if unknown).
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
    int readLen;
    while (totalRead < bufSize && (readLen = esp_http_client_read(client, buffer + totalRead, bufSize - totalRead)) >
                                       0)
    {
        totalRead += (size_t)readLen;
    }
    buffer[totalRead] = '\0';

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    // Skip ahead to the first '{' in case of any leading whitespace, the
    // same defensive step the quotables example takes.
    char *jsonStart = buffer;
    while (*jsonStart != '\0' && *jsonStart != '{')
        jsonStart++;

    cJSON *root = cJSON_Parse(jsonStart);
    free(buffer);

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse weather JSON");
        userInfo->apiError = true;
        return;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    cJSON *daily = cJSON_GetObjectItem(root, "daily");
    cJSON *hourly = cJSON_GetObjectItem(root, "hourly");
    cJSON *currentTime = current ? cJSON_GetObjectItem(current, "time") : NULL;

    if (!current || !daily || !hourly || !cJSON_IsString(currentTime))
    {
        ESP_LOGE(TAG, "Unexpected weather JSON structure");
        cJSON_Delete(root);
        userInfo->apiError = true;
        return;
    }

    userInfo->apiError = false;

    // --- "Now" (from the API's own local-time timestamp, see file header) ---
    const char *timeStr = cJSON_GetStringValue(currentTime);
    snprintf(userInfo->lastUpdated, sizeof(userInfo->lastUpdated), "%s", timeStr);
    extractDate(timeStr, userInfo->lastUpdatedDate, sizeof(userInfo->lastUpdatedDate));
    extractTime(timeStr, userInfo->lastUpdatedTime, sizeof(userInfo->lastUpdatedTime));

    const char *tPos = strchr(timeStr, 'T');
    userInfo->currentHour = tPos ? atoi(tPos + 1) : 0;

    // --- Current conditions ---
    weatherData->currentTemp = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(current, "temperature_2m"));
    weatherData->feelsLike = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(current, "apparent_temperature"));
    weatherData->precipitation = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(current, "precipitation"));
    weatherData->windSpeed = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(current, "wind_speed_10m"));
    weatherData->weatherCode = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(current, "weather_code"));
    weatherData->isDay = cJSON_GetNumberValue(cJSON_GetObjectItem(current, "is_day")) != 0;
    snprintf(weatherData->weatherDescription, sizeof(weatherData->weatherDescription), "%s",
             getWeatherDescription(weatherData->weatherCode));

    // --- 7-day forecast ---
    cJSON *dailyTimeArr = cJSON_GetObjectItem(daily, "time");
    cJSON *sunriseArr = cJSON_GetObjectItem(daily, "sunrise");
    cJSON *sunsetArr = cJSON_GetObjectItem(daily, "sunset");
    cJSON *uvArr = cJSON_GetObjectItem(daily, "uv_index_max");
    cJSON *minArr = cJSON_GetObjectItem(daily, "temperature_2m_min");
    cJSON *maxArr = cJSON_GetObjectItem(daily, "temperature_2m_max");
    cJSON *codeArr = cJSON_GetObjectItem(daily, "weather_code");

    cJSON *day0 = cJSON_GetArrayItem(dailyTimeArr, 0);
    int weekday0 = cJSON_IsString(day0) ? dayOfWeekFromDate(cJSON_GetStringValue(day0)) : 0;

    cJSON *sunrise0 = cJSON_GetArrayItem(sunriseArr, 0);
    if (cJSON_IsString(sunrise0))
        extractSun(cJSON_GetStringValue(sunrise0), weatherData->sunrise, sizeof(weatherData->sunrise));

    cJSON *sunset0 = cJSON_GetArrayItem(sunsetArr, 0);
    if (cJSON_IsString(sunset0))
        extractSun(cJSON_GetStringValue(sunset0), weatherData->sunset, sizeof(weatherData->sunset));

    weatherData->uvIndex = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(uvArr, 0));

    for (int i = 0; i < 7; i++)
    {
        weatherData->dailyMinTemp[i] = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(minArr, i));
        weatherData->dailyMaxTemp[i] = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(maxArr, i));
        weatherData->dailyWeatherCodes[i] = (int)cJSON_GetNumberValue(cJSON_GetArrayItem(codeArr, i));
        snprintf(weatherData->dailyNames[i], sizeof(weatherData->dailyNames[i]), "%s",
                 getDayName(weekday0, i));
    }

    // --- Next 6 hours (unused by the current GUI layouts, see WeatherData.h) ---
    cJSON *hTempArr = cJSON_GetObjectItem(hourly, "temperature_2m");
    cJSON *hPrecipArr = cJSON_GetObjectItem(hourly, "precipitation_probability");
    cJSON *hTimeArr = cJSON_GetObjectItem(hourly, "time");

    for (int i = 0; i < 6; i++)
    {
        int sourceIndex = userInfo->currentHour + i;
        weatherData->hourlyTemps[i] = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(hTempArr, sourceIndex));
        weatherData->hourlyPrecip[i] = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(hPrecipArr, sourceIndex));

        cJSON *hTime = cJSON_GetArrayItem(hTimeArr, sourceIndex);
        if (cJSON_IsString(hTime))
            extractSun(cJSON_GetStringValue(hTime), weatherData->hourlyTimes[i], sizeof(weatherData->hourlyTimes[i]));
    }

    cJSON_Delete(root);
}
