/**
 * @file        Gui.h
 * @author      Fran Fodor for Soldered
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 2.
 *
 * @details     Ported from the Inkplate2_OpenMeteo_Weather_Station Arduino
 *              example's src/Gui.h. The drawing calls themselves are
 *              Adafruit_GFX-compatible and are carried over almost verbatim;
 *              only the includes and the way the Inkplate instance is passed
 *              in change (Inkplate& constructor argument, same as the
 *              original).
 */

#pragma once

#include "Inkplate.h"
#include "Network.h"
#include "WeatherData.h"

class Gui
{
  public:
    Gui(Inkplate &inkplate);

    void drawBackground();

    // Layout 1: 3-day forecast strip (icons + min/max temperature arrows).
    void displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);

    // Layout 2: current city/temperature/condition summary.
    void displayWeatherData2(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);

    void wifiError();
    void apiError();

  private:
    Inkplate &inkplate;

    // Declared for parity with the original Gui.h; never implemented or
    // called there either (weatherData->hourly* is fetched but never
    // rendered by either layout) - kept unimplemented here for the same
    // reason.
    void drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);
    const uint8_t *getBatteryIcon(int percentage);

    const uint8_t *getWeatherIcon(int code);
};
