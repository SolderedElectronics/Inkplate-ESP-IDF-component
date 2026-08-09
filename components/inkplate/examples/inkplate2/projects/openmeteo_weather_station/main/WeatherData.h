/**
 * @file        WeatherData.h
 * @author      Fran Fodor for Soldered
 * @brief       Plain data container for the Open-Meteo weather dashboard.
 *
 * @details     Ported from the Inkplate2_OpenMeteo_Weather_Station Arduino
 *              example's WeatherData class. Arduino `String` members are
 *              replaced with fixed-size `char[]` buffers so the struct has no
 *              hidden heap allocations, matching this component's other
 *              ported examples.
 */

#pragma once

/**
 * @brief Holds the current conditions, 7-day forecast and 6-hour outlook
 *        fetched from the Open-Meteo API by NetworkFunctions::fetchWeatherData().
 */
class WeatherData
{
  public:
    // --- Current conditions ---
    float currentTemp = 0;
    float feelsLike = 0;
    char sunrise[8] = "??:??";
    char sunset[8] = "??:??";
    float uvIndex = 0;
    float precipitation = 0;
    float windSpeed = 0;
    int weatherCode = 0;
    char weatherDescription[24] = "Unknown condition";
    bool isDay = true;

    // --- 7-day forecast ---
    float dailyMinTemp[7] = {0};
    float dailyMaxTemp[7] = {0};
    int dailyWeatherCodes[7] = {0};
    char dailyNames[7][4] = {"?"};

    // --- Next 6 hours (unused by the current GUI layouts, kept for parity
    //     with the original sketch, which fetched but never displayed it) ---
    float hourlyTemps[6] = {0};
    float hourlyPrecip[6] = {0};
    char hourlyTimes[6][8] = {"??:??"};
};
