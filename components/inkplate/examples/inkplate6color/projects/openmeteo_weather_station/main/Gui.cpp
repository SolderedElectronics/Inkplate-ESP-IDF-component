/**
 * @file        Gui.cpp
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 6Color.
 *
 * @details     Ported from the Inkplate6COLOR_OpenMeteo_Weather_Station
 *              Arduino example's src/Gui.cpp. Drawing calls are unchanged
 *              (Adafruit_GFX-compatible); WeatherData/UserInfo fields are
 *              now fixed-size char buffers instead of Arduino `String`, but
 *              print()/println() accept `const char*` directly so the call
 *              sites did not need to change.
 *
 *              Color handling (the main real content difference from this
 *              board's grayscale siblings): Inkplate 6Color is a 7-color
 *              e-paper panel, so plain grayscale levels 0-7 don't apply
 *              here. The original sketch's numeric color arguments were
 *              already written against this board's palette
 *              (INKPLATE_BLACK=0, INKPLATE_WHITE=1, INKPLATE_GREEN=2,
 *              INKPLATE_BLUE=3, INKPLATE_RED=4, INKPLATE_YELLOW=5,
 *              INKPLATE_ORANGE=6) -- some of their inline comments were
 *              stale copy-paste from a grayscale sketch ("white color") but
 *              the numeric values are correct for this palette, so this
 *              port uses the matching INKPLATE_* macro at each call site and
 *              corrects the comments to describe what is actually drawn:
 *              black text/outlines on the white cleared background, a black
 *              panel with white text/icon for the battery/status box, and a
 *              red "hot" (max temp) / blue "cold" (min temp) accent used for
 *              both the temperature-graph labels and the weekly forecast's
 *              up/down temperature arrows. Green/yellow/orange are not used
 *              anywhere in this dashboard, matching the original sketch.
 *
 *              Layout values are unchanged from the original sketch, which
 *              already targeted Inkplate 6Color's native 600x448 panel. The
 *              weekly forecast loop only shows 6 days (not 7): the original
 *              sketch's comment says "7-day forecast" but the loop bound and
 *              `dayWidth` layout math are sized for 6 columns across the
 *              600px-wide panel (7 would overflow it), so this port keeps
 *              the loop at 6 iterations to match the original's actual
 *              (working) behavior rather than its comment.
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
#include "fonts/FreeSans24pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans32pt7b.h"

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate)
{
}

void Gui::drawBackground()
{
    // main info
    inkplate.drawRect(0, 0, 600, 110, INKPLATE_BLACK);

    // user info (filled black panel; text/icon drawn in white on top of it)
    inkplate.fillRect(440, 0, 160, 110, INKPLATE_BLACK);

    // graph
    inkplate.drawRect(0, 110, 600, 210, INKPLATE_BLACK);

    // weekly
    inkplate.drawRect(0, 320, 600, 128, INKPLATE_BLACK);
}

void Gui::wifiError()
{
    inkplate.clearDisplay();
    inkplate.setTextColor(INKPLATE_BLACK);
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
    inkplate.setTextColor(INKPLATE_BLACK);
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

// Weather icons based on the Open-Meteo weather_code.
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
    int graphX = 60;
    int graphY = 135;
    int graphWidth = 500;
    int graphHeight = 170;

    inkplate.setCursor(10, 135);
    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setTextColor(INKPLATE_BLACK);
    inkplate.print("Hourly temperature and precipitation");

    int marginX = 20;
    int marginY = 20;
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
    inkplate.drawLine(chartLeft, chartTop, chartLeft, chartBottom, INKPLATE_BLACK);                   // Y-axis
    inkplate.drawLine(chartLeft, chartBottom, graphX + graphWidth + 20, chartBottom, INKPLATE_BLACK); // X-axis

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

    // Draw Min temperature label in blue ("cold")
    inkplate.setTextColor(INKPLATE_BLUE);
    inkplate.setCursor(chartLeft - 70, yMin - 20);
    inkplate.print(paddedTempMin, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    // Draw Mid temperature label in black
    inkplate.setTextColor(INKPLATE_BLACK);
    inkplate.setCursor(chartLeft - 70, yMid);
    inkplate.print(paddedTempMid, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    // Draw Max temperature label in red ("hot")
    inkplate.setTextColor(INKPLATE_RED);
    inkplate.setCursor(chartLeft - 70, yMax + 20);
    inkplate.print(paddedTempMax, 1); // Show temperature with 1 decimal place
    inkplate.print(userInfo->temperatureLabel);

    inkplate.setTextColor(INKPLATE_BLACK);

    // Draw precipitation bars in blue
    for (int i = 0; i < 6; i++)
    {
        int x = chartLeft + (int)(i * xStep);
        int barHeight =
            (precipMax > 0) ? (int)((weatherData->hourlyPrecip[i] / precipMax) * (graphHeight - 2 * marginY)) : 0;
        int y = chartBottom - barHeight;

        // Draw the precipitation bar
        inkplate.fillRect(x + 5, y + 10, 10, barHeight - 10, INKPLATE_BLUE);

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

        inkplate.drawLine(x1, y1, x2, y2, INKPLATE_BLACK);
    }

    // Time labels under X-axis
    inkplate.setTextColor(INKPLATE_BLACK);
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
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(INKPLATE_BLACK);

    // City label icon, drawn in red as an accent color for this dashboard.
    inkplate.drawBitmap(10, 5, icon_s_gray, 48, 48, INKPLATE_RED);
    inkplate.setCursor(70, 40);
    inkplate.print(userInfo->city);

    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setCursor(70, 90);
    inkplate.print(weatherData->currentTemp);
    inkplate.print(userInfo->temperatureLabel);

    inkplate.setFont(&FreeSans12pt7b);
    inkplate.drawBitmap(10, 55, getWeatherIcon(weatherData->weatherCode), 48, 48, INKPLATE_BLACK);
    inkplate.setCursor(160, 90);
    inkplate.println(weatherData->weatherDescription);

    // Section 2: User Info and Battery (white text/icon on the black panel
    // drawn by drawBackground())

    batteryLevel = voltageToPercentage(voltage);

    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setTextColor(INKPLATE_WHITE);

    int yUser = 10;

    inkplate.drawBitmap(445, 0, getBatteryIcon(batteryLevel), 48, 48, INKPLATE_WHITE);

    yUser += 17;

    inkplate.setCursor(495, yUser + 2);
    inkplate.print(batteryLevel);
    inkplate.println("%");

    yUser += 33;
    inkplate.setCursor(445, yUser);
    inkplate.println(userInfo->lastUpdatedDate);

    yUser += 20;
    inkplate.setCursor(445, yUser);
    inkplate.print("Last refresh: ");
    inkplate.println(userInfo->lastUpdatedTime);

    yUser += 20;
    inkplate.setCursor(445, yUser);
    inkplate.println(userInfo->username);

    // Section 4: Weekly Forecast (only 6 of the 7 fetched days fit across
    // the 600px-wide panel; see the file-level comment above).
    inkplate.setTextColor(INKPLATE_WHITE);

    int startX = 18;                      // Starting x-position for the weekly forecast
    int startY = 345;                      // Starting y-position for the weekly forecast
    int iconSize = 48;                     // Size of the icon
    int margin = 5;                        // Margin between elements
    int dayWidth = iconSize + margin + 45; // Space for icon + margin + text width

    // Loop through the forecast and display each day
    for (int i = 0; i < 6; i++)
    {
        inkplate.setFont(&FreeSans12pt7b);
        int xPos = startX + i * dayWidth;

        // Day name
        inkplate.setCursor(xPos + 15, startY);
        inkplate.setTextColor(INKPLATE_BLACK);
        inkplate.print(weatherData->dailyNames[i]);

        // Weather icon
        inkplate.setFont(&FreeSans9pt7b);
        inkplate.drawBitmap(xPos + 15, startY + 10, getWeatherIcon(weatherData->dailyWeatherCodes[i]), iconSize,
                            iconSize, INKPLATE_BLACK);
        int tempYStart = startY + 10 + iconSize + margin + 5;

        // === Max Temp - Up Arrow Triangle (red = "hot") ===
        int arrowX = xPos;
        int arrowY = tempYStart + 5;
        // Triangle pointing up
        inkplate.fillTriangle(arrowX, arrowY,         // bottom center
                              arrowX - 4, arrowY + 6, // bottom left
                              arrowX + 4, arrowY + 6, // bottom right
                              INKPLATE_RED);
        // Max temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMaxTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
        // === Min Temp - Down Arrow Triangle (blue = "cold") ===
        arrowY += 20;
        // Triangle pointing down
        inkplate.fillTriangle(arrowX, arrowY + 6, // top center
                              arrowX - 4, arrowY, // bottom left
                              arrowX + 4, arrowY, // bottom right
                              INKPLATE_BLUE);
        // Min temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 6);
        inkplate.print(weatherData->dailyMinTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
    }

    // Section 6: Graph info
    drawTemperaturePrecipGraph(weatherData, userInfo);

    // Finalize drawing
    inkplate.display();
}
