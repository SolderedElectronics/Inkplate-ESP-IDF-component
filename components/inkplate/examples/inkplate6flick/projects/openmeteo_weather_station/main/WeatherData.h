/**
 * @file        WeatherData.h
 * @brief       Plain data holder for one Open-Meteo forecast fetch (current
 *              conditions + 7-day daily forecast + 6-hour hourly forecast).
 *
 * @details     Ported from the Inkplate6FLICK_OpenMeteo_Weather_Station
 *              Arduino example's src/WeatherData.h. Arduino `String`
 *              members are replaced with fixed-size char buffers.
 */

#pragma once

class WeatherData
{
  public:
    float currentTemp;
    float feelsLike;
    char sunrise[8];
    char sunset[8];
    float uvIndex;
    float precipitation;
    float windSpeed;
    float dailyMinTemp[7];
    float dailyMaxTemp[7];
    char dailyNames[7][4];
    char weatherDescription[32];
    int weatherCode;
    int dailyWeatherCodes[7];
    bool isDay;
    float hourlyTemps[6];
    float hourlyPrecip[6];
    char hourlyTimes[6][8];
};
