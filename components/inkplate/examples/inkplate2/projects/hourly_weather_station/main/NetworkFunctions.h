/**
 * NetworkFunctions.h
 * Inkplate ESP-IDF component - Inkplate 2 hourly weather station example
 *
 * Ported from the Inkplate-Arduino-library Inkplate2_Hourly_Weather_Station
 * example. Fetches OpenWeatherMap "onecall" forecast data over HTTPS and
 * exposes the fields the UI needs (three temperatures + three weather icon
 * abbreviations + the corresponding hour labels).
 *
 * Original Arduino version:
 * David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek
 * https://github.com/e-radionicacom/Inkplate-6-Arduino-library
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 * This code is released under the GNU Lesser General Public License v3.0.
 */

#pragma once

#include <time.h>

#include "WiFi.h"

/**
 * @brief Fetches and stores OpenWeatherMap hourly forecast data.
 *
 * @note Unlike the original Arduino version, WiFi connection itself is not
 *       handled here (see main.cpp, which uses display.wifi directly) -
 *       this class only performs the HTTPS request + JSON parsing.
 */
class NetworkFunctions
{
  public:
    /**
     * @brief Download and parse the OpenWeatherMap "onecall" forecast.
     *
     * @param wifi   Reference to an already-connected WiFi instance
     *               (display.wifi), used to perform the HTTPS download.
     * @param lon    Longitude string, e.g. "18.5947808".
     * @param lat    Latitude string, e.g. "45.5510548".
     * @param apiKey OpenWeatherMap API key.
     * @param temp1  Out: current temperature, formatted as a short string.
     * @param temp2  Out: temperature one hour from now.
     * @param temp3  Out: temperature two hours from now.
     * @param abbr1  Out: weather icon abbreviation for temp1 (e.g. "01d").
     * @param abbr2  Out: weather icon abbreviation for temp2.
     * @param abbr3  Out: weather icon abbreviation for temp3.
     * @return bool true on success, false if the request or parsing failed.
     */
    bool getData(WiFi &wifi, const char *lon, const char *lat, const char *apiKey, char *temp1, char *temp2,
                 char *temp3, char *abbr1, char *abbr2, char *abbr3);

    /**
     * @brief Formats the "now"/"+1h"/"+2h" hour labels from the last
     *        successful getData() call, e.g. "14h".
     *
     * @param hour1 Out: current hour label.
     * @param hour2 Out: next hour label.
     * @param hour3 Out: hour after next label.
     */
    void getHours(char *hour1, char *hour2, char *hour3);

    // Timestamp ("dt") of the last successfully fetched data point, used by
    // getHours() to derive the hour labels.
    time_t dataEpoch = 0;

  private:
    // Timezone offset (in whole hours) for the requested location, as
    // reported by the API ("timezone_offset" is in seconds).
    int timeZone = 0;
};
