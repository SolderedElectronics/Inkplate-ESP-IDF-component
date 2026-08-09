/**
 * @file        Gui.h
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 6 FLICK.
 *
 * @details     Ported from the Inkplate6FLICK_OpenMeteo_Weather_Station
 *              Arduino example's src/Gui.h. The drawing calls themselves are
 *              Adafruit_GFX-compatible (setCursor/setFont/print/fillRect/
 *              drawBitmap/...) and are unchanged from the original; only the
 *              WeatherData/UserInfo field types changed (Arduino `String`
 *              -> fixed-size char buffers).
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
    void displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);
    void wifiError();
    void apiError();

    int batteryLevel;
    double voltage;

  private:
    Inkplate &inkplate;

    void drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);
    const uint8_t *getWeatherIcon(int code);
    const uint8_t *getBatteryIcon(int percentage);
    int voltageToPercentage(double voltage);
};
