/**
 * @file        Network.h
 * @brief       Fetches current/daily/hourly forecast data from the free
 *              Open-Meteo API (no API key required) and parses it with
 *              cJSON.
 *
 * @details     Ported from the Inkplate6_OpenMeteo_Weather_Station Arduino
 *              example's src/Network.h. The original used
 *              WiFi.h/HTTPClient + ArduinoJson and Arduino's
 *              configTime()/getLocalTime(); this version uses
 *              esp_http_client (with the ESP-IDF certificate bundle for TLS
 *              verification) together with cJSON and esp_sntp for time
 *              sync. WiFi connection itself is handled by display.wifi in
 *              main.cpp, matching the rest of this component's examples.
 *              Arduino `String` fields/return values are replaced with
 *              fixed-size char buffers filled via snprintf().
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

#include "WeatherData.h"
#include <cstddef>

/**
 * @brief Fetches and parses Open-Meteo forecast data.
 */
class NetworkFunctions
{
  public:
    /**
     * @brief Mirrors the original Arduino UserInfo struct, with `String`
     *        fields replaced by fixed-size char buffers.
     */
    struct UserInfo
    {
        char lastUpdated[24];      // "YYYY-MM-DD HH:MM"
        int currentHour;           // Local hour of day (0-23), or -1 if unset.
        char city[32];             // Display-only city label (caller-supplied).
        char username[32];         // Display-only username label (caller-supplied).
        char lastUpdatedDate[16];  // "YYYY-MM-DD"
        char lastUpdatedTime[8];   // "HH:MM"
        bool apiError;             // True if the last fetchWeatherData() call failed.
        bool useMetric;            // Caller-supplied: true = Celsius/km-h, false = Fahrenheit/mph.
        char temperatureLabel[4];  // " C" or " F"
        char speedLabel[8];        // " km/h" or " mph"
    };

    /**
     * @brief Synchronize the system clock via NTP and apply a fixed UTC
     *        offset (no DST handling), equivalent to the original sketch's
     *        `configTime(timeZone * 3600, 0, ntpServer)` call.
     *
     * @param utcOffsetHours UTC offset of the target location, in hours
     *                        (e.g. 2 for UTC+2, -4 for UTC-4).
     * @param ntpServer NTP pool/server hostname to sync against.
     * @return true if the clock was synchronized before the retry budget
     *         ran out, false otherwise (the TZ offset is applied either
     *         way).
     */
    bool setupTime(int utcOffsetHours, const char *ntpServer);

    /**
     * @brief Fetch current/daily/hourly weather data for the given
     *        coordinates from the Open-Meteo API.
     *
     * @param weatherData Out: parsed forecast data (untouched on failure).
     * @param userInfo In/out: `useMetric` must already be set by the
     *                 caller; on return, timestamps/unit labels/apiError
     *                 are filled in. `city`/`username` are not touched.
     * @param latitude Latitude of the target location.
     * @param longitude Longitude of the target location.
     */
    void fetchWeatherData(WeatherData *weatherData, UserInfo *userInfo, float latitude, float longitude);

  private:
    const char *getWeatherDescription(int code);
    void extractDate(const char *dateTime, char *out, size_t outSize);
    void extractTime(const char *dateTime, char *out, size_t outSize);
    void extractSun(const char *dateTime, char *out, size_t outSize);
    void getFormattedTime(char *out, size_t outSize);
    int getCurrentHour();
    const char *getDayName(int dayIndex);
};
