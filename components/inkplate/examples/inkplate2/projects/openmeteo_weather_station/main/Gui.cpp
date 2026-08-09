/**
 * @file        Gui.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 2.
 *
 * @details     Ported from the Inkplate2_OpenMeteo_Weather_Station Arduino
 *              example's src/Gui.cpp. The drawing calls are
 *              Adafruit_GFX-compatible and carried over almost unchanged;
 *              `String` concatenation is replaced with direct `print()`
 *              calls on the fixed-size buffers/values already stored in
 *              WeatherData/UserInfo.
 */

#include "Gui.h"

#include "Network.h"
#include "WeatherData.h"

// all the weather icons
#include "binary_Icons/icon_s_clear_sky.h"
#include "binary_Icons/icon_s_fog.h"
#include "binary_Icons/icon_s_gray.h"
#include "binary_Icons/icon_s_moon.h"
#include "binary_Icons/icon_s_partly_cloudy.h"
#include "binary_Icons/icon_s_rain.h"
#include "binary_Icons/icon_s_snow.h"
#include "binary_Icons/icon_s_storm.h"
#include "binary_Icons/icon_s_thermometer.h"

// font
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans6pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate)
{
}

void Gui::drawBackground()
{
}

void Gui::wifiError()
{
    inkplate.clearDisplay();
    inkplate.setTextColor(INKPLATE2_BLACK);
    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setCursor(10, 30);
    inkplate.print("WiFi connection failed.");
    inkplate.setCursor(10, 80);
    inkplate.print("Check credentials or try again.");
    inkplate.display();
}

void Gui::apiError()
{
    inkplate.clearDisplay();
    inkplate.setTextColor(INKPLATE2_BLACK);
    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setCursor(10, 30);
    inkplate.print("HTTP request failed.");
    inkplate.setCursor(10, 80);
    inkplate.print("Check API URL or try again.");
    inkplate.display();
}

// Weather Icons based on Open-Meteo (WMO) weather code
const uint8_t *Gui::getWeatherIcon(int code)
{
    switch (code)
    {
    case 0:
        return icon_s_clear_sky;
    case 1:
    case 2:
    case 3:
        return icon_s_partly_cloudy;
    case 45:
    case 48:
        return icon_s_fog;
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
    case 61:
    case 63:
    case 65:
    case 66:
    case 67:
    case 80:
    case 81:
    case 82:
        return icon_s_rain;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return icon_s_snow;
    case 95:
    case 96:
    case 99:
        return icon_s_storm;
    default:
        return icon_s_gray;
    }
}

// --- Layout 1: 3-day forecast strip ---
void Gui::displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    inkplate.setTextColor(INKPLATE2_BLACK);
    int startX = 10;                       // Starting x-position for the weekly forecast
    int startY = 15;                       // Starting y-position for the weekly forecast
    int iconSize = 48;                     // Size of the icon
    int margin = 5;                        // Margin between elements
    int dayWidth = iconSize + margin + 15; // Space for icon + margin + text width
    // Loop through the first 3 days of the forecast and display each day
    for (int i = 0; i < 3; i++)
    {
        inkplate.setFont(&FreeSans6pt7b);
        int xPos = startX + i * dayWidth;
        // Day name
        inkplate.setCursor(xPos + 15, startY);
        inkplate.print(weatherData->dailyNames[i]);
        // Weather icon
        inkplate.setFont(&FreeSans6pt7b);
        inkplate.drawBitmap(xPos + 5, startY + 8, getWeatherIcon(weatherData->dailyWeatherCodes[i]), iconSize,
                            iconSize, INKPLATE2_BLACK);
        int tempYStart = startY + iconSize + margin;
        // === Max Temp - Up Arrow Triangle ===
        int arrowX = xPos;
        int arrowY = tempYStart + 15;
        // Triangle pointing up
        inkplate.fillTriangle(arrowX, arrowY,         // bottom center
                              arrowX - 4, arrowY + 6, // bottom left
                              arrowX + 4, arrowY + 6, // bottom right
                              INKPLATE2_RED);
        // Max temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMaxTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
        // === Min Temp - Down Arrow Triangle ===
        arrowY += 10;
        // Triangle pointing down
        inkplate.fillTriangle(arrowX, arrowY + 6, // top center
                              arrowX - 4, arrowY, // bottom left
                              arrowX + 4, arrowY, // bottom right
                              INKPLATE2_BLACK);
        // Min temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMinTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
    }

    // Finalize drawing
    inkplate.display();
}

// --- Layout 2: current city/temperature/condition summary ---
void Gui::displayWeatherData2(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setTextColor(INKPLATE2_BLACK);
    inkplate.drawBitmap(0, 5, icon_s_gray, 48, 48, INKPLATE2_RED);
    inkplate.setCursor(55, 35);
    inkplate.print(userInfo->city);
    inkplate.setFont(&FreeSans9pt7b);
    // inkplate.drawBitmap(0, 50, icon_s_thermometer, 24, 24, INKPLATE2_BLACK);
    inkplate.setCursor(55, 75);
    inkplate.print(weatherData->currentTemp);
    inkplate.print(userInfo->temperatureLabel);
    inkplate.drawBitmap(0, 55, getWeatherIcon(weatherData->weatherCode), 48, 48, INKPLATE2_BLACK);
    inkplate.setCursor(55, 95);
    inkplate.println(weatherData->weatherDescription);

    // Finalize drawing
    inkplate.display();
}
