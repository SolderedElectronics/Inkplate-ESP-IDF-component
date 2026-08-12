/**
 * @file        Network.h
 * @author      Fran Fodor for Soldered
 * @brief       Fetches current conditions + forecast from the Open-Meteo API.
 *
 * @details     Ported from the Inkplate2_OpenMeteo_Weather_Station Arduino
 *              example's src/Network.h. The original used HTTPClient +
 *              ArduinoJson and derived "now" (for indexing the hourly array
 *              and naming forecast days) from the ESP32's NTP-synced clock.
 *              This port instead uses esp_http_client + cJSON (this
 *              component's established technique, see
 *              projects/quotables/main/NetworkFunctions.cpp) and reads "now"
 *              directly out of the API response's own `current.time` and
 *              `daily.time[0]` fields, so no NTP/RTC dependency is needed at
 *              all (see Notes in README.md).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

#include <cstddef>

#include "WeatherData.h"

/**
 * @brief Downloads and parses the Open-Meteo "forecast" endpoint.
 */
class NetworkFunctions
{
  public:
    /**
     * @brief User/display preferences, plus the bits of state derived from
     *        the most recent successful fetch (current hour, last-updated
     *        timestamp, unit labels).
     */
    struct UserInfo
    {
        char lastUpdated[24];     // e.g. "2025-04-08T14:00" (API "current.time")
        int currentHour;          // Hour-of-day at the requested location, 0-23
        char city[32];            // Display label, set by main.cpp
        char username[32];        // Display label, set by main.cpp (unused by
                                   // the current GUI layouts, kept for parity)
        char lastUpdatedDate[16]; // e.g. "2025-04-08"
        char lastUpdatedTime[8];  // e.g. "14:00"
        bool apiError;
        bool useMetric;
        char temperatureLabel[4]; // " C" or " F"
        char speedLabel[8];       // " km/h" or " mph"
    };

    /**
     * @brief Fetch current conditions, the 7-day forecast and the next 6
     *        hours for (latitude, longitude) and store the result in
     *        `weatherData`. On failure, `userInfo->apiError` is set to true
     *        and `weatherData`/`userInfo` are left with their previous (or
     *        default) values.
     *
     * @param weatherData Out: parsed weather data.
     * @param userInfo In/out: `useMetric` selects units on input; on output
     *                 `apiError`, `currentHour`, `lastUpdated*` and the unit
     *                 labels are filled in.
     * @param latitude Latitude of the location to fetch weather for.
     * @param longitude Longitude of the location to fetch weather for.
     */
    void fetchWeatherData(WeatherData *weatherData, UserInfo *userInfo, float latitude, float longitude);

  private:
    // --- Private helper methods (mirroring the original Network.cpp) ---
    const char *getWeatherDescription(int code);
    void extractDate(const char *dateTime, char *out, size_t outSize);
    void extractTime(const char *dateTime, char *out, size_t outSize);
    void extractSun(const char *dateTime, char *out, size_t outSize);
    const char *getDayName(int weekday0, int dayIndex);
};
