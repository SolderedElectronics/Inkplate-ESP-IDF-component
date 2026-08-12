/**
 * @file        Gui.cpp
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 6 FLICK.
 *
 * @details     Ported from the Inkplate6FLICK_OpenMeteo_Weather_Station
 *              Arduino example's src/Gui.cpp. Drawing calls are unchanged
 *              (Adafruit_GFX-compatible); WeatherData/UserInfo fields are
 *              now fixed-size char buffers instead of Arduino `String`, but
 *              print()/println() accept `const char*` directly so the call
 *              sites did not need to change. Color values 0 (black) through
 *              7 (white) are the same 3-bit grayscale levels used by the
 *              original sketch's INKPLATE_3BIT mode.
 *
 *              Layout coordinates are unchanged from the original sketch.
 *              They were checked against this board's native 1024x758
 *              landscape canvas (see components/inkplate/include/boards/
 *              Inkplate6.h): the graph/add-info split `fillRect(224, 200,
 *              800, 400, 7)` + `fillRect(0, 200, 224, 400, 0)` sums to
 *              exactly x=1024, confirming the sketch targets this board's
 *              native landscape orientation directly (no setRotation()
 *              needed, same conclusion as the Inkplate6 sibling port, but
 *              re-verified independently for this board's own coordinates
 *              rather than assumed). Two minor pre-existing overflows carried
 *              over unmodified from the original sketch: the top-right
 *              battery panel `fillRect(675, 0, 355, 200, 0)` reaches x=1030
 *              (6px past the 1024px-wide panel), and the weekly-forecast
 *              strip `fillRect(224, 600, 800, 170, 0)` reaches y=770 (12px
 *              past the 758px-tall panel). Both are silently clipped on
 *              real hardware; this is the same class of pre-existing layout
 *              quirk documented (not fixed) in the Inkplate6 sibling's
 *              Gui.cpp, just smaller in magnitude here, so this file keeps
 *              the coordinates verbatim to match upstream.
 */

#include "Gui.h"
#include "Network.h"
#include "WeatherData.h"
#include <cstdlib>

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

// all the battery icons
#include "binary_Icons/icon_s_full_battery.h"
#include "binary_Icons/icon_s_half_battery.h"
#include "binary_Icons/icon_s_high_battery.h"
#include "binary_Icons/icon_s_low_battery.h"

// fonts
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate)
{
}

void Gui::drawBackground()
{
    // main info
    inkplate.fillRect(0, 0, 800, 200, 7);
    inkplate.drawRect(0, 0, 800, 200, 0);

    // battery info
    inkplate.fillRect(675, 0, 355, 200, 0);

    // add info
    inkplate.fillRect(0, 200, 224, 400, 0);

    // graph info
    inkplate.fillRect(224, 200, 800, 400, 7);
    inkplate.drawRect(224, 200, 800, 400, 0);

    // weekly
    inkplate.fillRect(224, 600, 800, 170, 0);
}

void Gui::wifiError()
{
    inkplate.clearDisplay();
    inkplate.setTextColor(0);
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setCursor(50, 150);
    inkplate.print("WiFi connection failed.");
    inkplate.setCursor(50, 200);
    inkplate.print("Check credentials or try again.");
    inkplate.display();
}

void Gui::apiError()
{
    inkplate.clearDisplay();
    inkplate.setTextColor(0);
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setCursor(50, 150);
    inkplate.print("HTTP request failed.");
    inkplate.setCursor(50, 200);
    inkplate.print("Check API URL or try again.");
    inkplate.display();
}

int Gui::voltageToPercentage(double voltage)
{
    if (voltage >= 4.2)
        return 100;
    if (voltage <= 3.0)
        return 0;

    // Simple linear approximation
    return (int)(((voltage - 3.0) / (4.2 - 3.0)) * 100);
}

// Weather Icons based on open-meteo api code
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

const uint8_t *Gui::getBatteryIcon(int percentage)
{
    if (percentage >= 75)
        return icon_s_full_battery;
    else if (percentage >= 50)
        return icon_s_high_battery;
    else if (percentage >= 25)
        return icon_s_half_battery;
    else
        return icon_s_low_battery;
}

// --- Draw Temperature & Precipitation Graph ---
void Gui::drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    // Layout values for graph placement
    int graphX = 275;
    int graphY = 245;
    int graphWidth = 700;
    int graphHeight = 360;

    inkplate.setCursor(275, 255);
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setTextColor(0);
    inkplate.print("Hourly temperature and precipitation");

    int marginX = 40;
    int marginY = 40;
    int chartLeft = graphX + marginX;
    int chartBottom = graphY + graphHeight - marginY;
    int chartTop = graphY + marginY;
    float actualTempMin = 100, actualTempMax = -100;
    float precipMax = 100;

    // Find actual min/max for temperature and precipitation
    for (int i = 0; i < 6; i++)
    {
        if (weatherData->hourlyTemps[i] < actualTempMin)
            actualTempMin = weatherData->hourlyTemps[i];
        if (weatherData->hourlyTemps[i] > actualTempMax)
            actualTempMax = weatherData->hourlyTemps[i];
    }

    // Add padding but ensure min temperature doesn't go below zero (or another reasonable value)
    float paddedTempMin = actualTempMin - 2;
    float paddedTempMax = actualTempMax + 2;

    // Calculate the mid temperature as the average of min and max
    float paddedTempMid = (paddedTempMin + paddedTempMax) / 2;

    float tempRange = paddedTempMax - paddedTempMin;
    if (tempRange == 0)
        tempRange = 1; // Avoid division by zero
    float xStep = (graphWidth - 2 * marginX) / 5.0f;

    // Draw axes
    inkplate.drawLine(chartLeft, chartTop, chartLeft, chartBottom, 0);                   // Y-axis
    inkplate.drawLine(chartLeft, chartBottom, graphX + graphWidth + 20, chartBottom, 0); // X-axis

    // Calculate Y positions for min, mid, and max temperatures
    int yMin = chartBottom - (int)(((paddedTempMin - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY));
    int yMax = chartBottom - (int)(((paddedTempMax - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY));
    int yMid = chartBottom - (int)(((paddedTempMid - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY));

    // Ensure at least 20px gap between the min, mid, and max temperature labels
    if (abs(yMax - yMin) < 20)
    {
        int offset = 20 - abs(yMax - yMin); // Calculate the gap required
        yMax = yMin + offset;               // Adjust the max label position to create the gap
    }

    if (abs(yMid - yMin) < 20)
    {
        int offset = 20 - abs(yMid - yMin); // Calculate the gap required
        yMid = yMin + offset;               // Adjust the mid label position to create the gap
    }

    if (abs(yMax - yMid) < 20)
    {
        int offset = 20 - abs(yMax - yMid); // Calculate the gap required
        yMax = yMid + offset;               // Adjust the max label position to create the gap
    }

    // Draw the temperature labels (Min, Mid, and Max)
    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setTextColor(0);

    // Draw Min temperature label
    inkplate.setCursor(chartLeft - 70, yMin - 20);
    inkplate.print(paddedTempMin, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    // Draw Mid temperature label
    inkplate.setCursor(chartLeft - 70, yMid);
    inkplate.print(paddedTempMid, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    // Draw Max temperature label
    inkplate.setCursor(chartLeft - 70, yMax + 20);
    inkplate.print(paddedTempMax, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    // Draw precipitation bars
    for (int i = 0; i < 6; i++)
    {
        int x = chartLeft + (int)(i * xStep);
        int barHeight =
            (precipMax > 0) ? (int)((weatherData->hourlyPrecip[i] / precipMax) * (graphHeight - 2 * marginY)) : 0;
        int y = chartBottom - barHeight;

        // Draw the precipitation bar
        inkplate.fillRect(x + 5, y + 10, 10, barHeight - 10, 4);

        // Draw precipitation value on top of the bar
        inkplate.setCursor(x + 5, y - 5);
        inkplate.print(weatherData->hourlyPrecip[i], 0); // Show precipitation with 0 decimal places
        inkplate.print("%");
    }

    // Draw temperature line
    for (int i = 0; i < 5; i++)
    {
        int x1 = chartLeft + (int)(i * xStep);
        int x2 = chartLeft + (int)((i + 1) * xStep);

        int y1 = chartBottom - (int)(((weatherData->hourlyTemps[i] - paddedTempMin) / tempRange) *
                                     (graphHeight - 2 * marginY));
        int y2 = chartBottom - (int)(((weatherData->hourlyTemps[i + 1] - paddedTempMin) / tempRange) *
                                     (graphHeight - 2 * marginY));

        inkplate.drawLine(x1, y1, x2, y2, 0);
    }

    // Time labels under X-axis
    inkplate.setTextColor(0);
    for (int i = 0; i < 6; i++)
    {
        int x = chartLeft + (int)(i * xStep);
        inkplate.setCursor(x + 3, chartBottom + 20);
        inkplate.print(weatherData->hourlyTimes[i]);
    }
}

// --- Display All Weather Data ---
void Gui::displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    // Section 1: Main info
    inkplate.setFont(&FreeSansBold24pt7b);
    inkplate.setTextColor(0);

    inkplate.drawBitmap(25, 25, icon_s_gray, 48, 48, 0);
    inkplate.setCursor(100, 60);
    inkplate.print(userInfo->city);

    inkplate.setFont(&FreeSans18pt7b);
    inkplate.drawBitmap(25, 85, icon_s_thermometer, 48, 48, 0);
    inkplate.setCursor(100, 120);
    inkplate.print(weatherData->currentTemp);
    inkplate.print(userInfo->temperatureLabel);

    inkplate.drawBitmap(25, 145, getWeatherIcon(weatherData->weatherCode), 48, 48, 0);
    inkplate.setCursor(100, 185);
    inkplate.println(weatherData->weatherDescription);

    // Section 2: User Info and Battery

    batteryLevel = voltageToPercentage(voltage);

    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setTextColor(7);

    int yUser = 35;

    inkplate.drawBitmap(700, 5, getBatteryIcon(batteryLevel), 48, 48, 7);

    inkplate.setCursor(750, yUser);
    inkplate.print(batteryLevel);
    inkplate.println("%");

    yUser += 45;
    inkplate.setCursor(700, yUser);
    inkplate.println(userInfo->lastUpdatedDate);

    yUser += 45;
    inkplate.setCursor(700, yUser);
    inkplate.print("Last refresh: ");
    inkplate.println(userInfo->lastUpdatedTime);

    yUser += 45;
    inkplate.setCursor(700, yUser);
    inkplate.println(userInfo->username);

    // Section 3: Additional Info (feels like, wind, etc.)
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setTextColor(7);

    int y = 255;
    int xPos = 10;
    int yGap = 60;

    inkplate.setCursor(xPos, y);
    inkplate.print("Feels like: ");
    inkplate.print(weatherData->feelsLike);
    inkplate.print(userInfo->temperatureLabel);

    y += yGap;
    inkplate.setCursor(xPos, y);
    inkplate.print("Sunrise: ");
    inkplate.println(weatherData->sunrise);

    y += yGap;
    inkplate.setCursor(xPos, y);
    inkplate.print("Sunset: ");
    inkplate.println(weatherData->sunset);

    y += yGap;
    inkplate.setCursor(xPos, y);
    inkplate.print("UV index: ");
    inkplate.println(weatherData->uvIndex);

    y += yGap;
    inkplate.setCursor(xPos, y);
    inkplate.print("Wind: ");
    inkplate.print(weatherData->windSpeed);
    inkplate.print(userInfo->speedLabel);

    y += yGap;
    inkplate.setCursor(xPos, y);
    inkplate.print("Precip.: ");
    inkplate.print(weatherData->precipitation);
    inkplate.print(" %");

    // Section 4: Weekly Forecast
    inkplate.setTextColor(7);

    int startX = 240;                      // Starting x-position for the weekly forecast
    int startY = 630;                      // Starting y-position for the weekly forecast
    int iconSize = 48;                     // Size of the icon
    int margin = 5;                        // Margin between elements
    int dayWidth = iconSize + margin + 63; // Space for icon + margin + text width

    // Loop through the 7-day forecast and display each day
    for (int i = 0; i < 7; i++)
    {
        inkplate.setFont(&FreeSans12pt7b);
        int xPos2 = startX + i * dayWidth;

        // Day name
        inkplate.setCursor(xPos2 + 15, startY);
        inkplate.setTextColor(7);
        inkplate.print(weatherData->dailyNames[i]);

        // Weather icon
        inkplate.setFont(&FreeSans9pt7b);
        inkplate.drawBitmap(xPos2 + 15, startY + 20, getWeatherIcon(weatherData->dailyWeatherCodes[i]), iconSize,
                            iconSize, 7);
        int tempYStart = startY + 20 + iconSize + margin + 5;

        // === Max Temp - Up Arrow Triangle ===
        int arrowX = xPos2;
        int arrowY = tempYStart + 5;
        // Triangle pointing up
        inkplate.fillTriangle(arrowX, arrowY,         // bottom center
                              arrowX - 4, arrowY + 6, // bottom left
                              arrowX + 4, arrowY + 6, // bottom right
                              7                       // white color
        );
        // Max temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMaxTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
        // === Min Temp - Down Arrow Triangle ===
        arrowY += 20;
        // Triangle pointing down
        inkplate.fillTriangle(arrowX, arrowY + 6, // top center
                              arrowX - 4, arrowY, // bottom left
                              arrowX + 4, arrowY, // bottom right
                              7                   // white color
        );
        // Min temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMinTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
    }

    // Section 5: Day or Night indicator
    int iconX = 25;
    int iconY = 660;
    if (weatherData->isDay)
    {
        inkplate.fillRect(0, 600, 224, 150, 7);
        // inkplate.drawRect(0, 600, 224, 150, 0);
        inkplate.drawBitmap(iconX, iconY, icon_s_clear_sky, 48, 48, 0);
        inkplate.setCursor(iconX + 60, iconY + 35);
        inkplate.setTextColor(0);
        inkplate.setFont(&FreeSans12pt7b);
        inkplate.print("Daytime");
    }
    else
    {
        inkplate.fillRect(0, 600, 224, 150, 7);
        // inkplate.drawRect(0, 600, 224, 150, 0);
        inkplate.drawBitmap(iconX, iconY, icon_s_moon, 48, 48, 7);
        inkplate.setCursor(iconX + 60, iconY + 35);
        inkplate.setTextColor(7);
        inkplate.setFont(&FreeSans12pt7b);
        inkplate.print("Nighttime");
    }

    // Section 6: Graph info
    drawTemperaturePrecipGraph(weatherData, userInfo);

    // Finalize drawing
    inkplate.display();
}
