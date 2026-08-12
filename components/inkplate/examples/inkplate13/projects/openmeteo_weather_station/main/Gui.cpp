/**
 * @file        Gui.cpp
 * @brief       Draws the Open-Meteo weather dashboard on Inkplate 13SPECTRA.
 *
 * @details     Ported from the Inkplate13SPECTRA_OpenMeteo_Weather_Station
 *              Arduino example's src/Gui.cpp. Drawing calls are unchanged
 *              (Adafruit_GFX-compatible); WeatherData/UserInfo fields are
 *              now fixed-size char buffers instead of Arduino `String`, but
 *              print()/println() accept `const char*` directly so the call
 *              sites did not need to change. All layout coordinates below
 *              are taken directly from the original Inkplate13SPECTRA
 *              sketch (which already targets this board's native 1600x1200
 *              landscape canvas) -- they are NOT a rescaled copy of the
 *              Inkplate6Color port's much smaller 600x448 layout. The two
 *              sketches share the same overall sections (main info /
 *              battery+status / weekly forecast / hourly graph) but
 *              Inkplate 13SPECTRA's far larger canvas also has room for two
 *              sections the Inkplate 6Color version omits: a dedicated
 *              "additional info" panel (feels like / sunrise / sunset / UV
 *              index / wind / precipitation) and a day/night indicator.
 *
 *              Color handling (the main real content difference from this
 *              board's grayscale siblings): Inkplate 13SPECTRA is a 6-color
 *              e-paper panel -- black/white/yellow/red/blue/green, see
 *              boards/Inkplate13.h -- there is NO orange on this board,
 *              unlike Inkplate 6Color's 7-color palette (which adds orange).
 *              The original sketch's numeric color arguments (0, 1, 3, 4)
 *              are replaced here with the matching INKPLATE_* macro at each
 *              call site: 0 -> INKPLATE_BLACK, 1 -> INKPLATE_WHITE, and
 *              3 -> INKPLATE_RED for the "hot"/max-temperature accent
 *              (matches this board's real palette, where INKPLATE_RED is
 *              also index 3). The remaining literal, 4, is used by the
 *              original sketch for the "cold"/min-temperature and
 *              precipitation accent -- but index 4 is a gap in this board's
 *              palette (INKPLATE_BLACK=0, WHITE=1, YELLOW=2, RED=3, BLUE=5,
 *              GREEN=6; see boards/Inkplate13.h), so it cannot be a literal
 *              match. Mapped to INKPLATE_BLUE here instead, both because
 *              that is the semantic opposite of the red "hot" accent used
 *              right next to it (mirroring the identical red/blue hot/cold
 *              convention this component's Inkplate6Color OpenMeteo port
 *              uses) and because no other used color in this file is a
 *              plausible match. No orange/green/yellow is used anywhere in
 *              this dashboard, matching the original sketch.
 *
 *              Black-filled panels (status/battery, additional info, and
 *              weekly forecast) get white text/icons/arrows drawn on top of
 *              them; everywhere else uses black text/icons on the cleared
 *              white background. Unlike the Inkplate6Color port, this
 *              board's weekly forecast panel is filled black too, so its
 *              up/down temperature arrows are white rather than red/blue --
 *              only the hourly graph's labels and precipitation bars carry
 *              the red/blue temperature accent on this board.
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
    // Main info: city, temperature, weather description (outline only).
    inkplate.drawRect(0, 0, 1600, 291, INKPLATE_BLACK);

    // Battery info, last refresh, username (filled black panel; text/icon
    // drawn in white on top of it).
    inkplate.fillRect(1033, 0, 567, 291, INKPLATE_BLACK);

    // Additional weather info: feels like / sunrise / sunset / UV / wind /
    // precipitation (filled black panel; text drawn in white on top of it).
    inkplate.fillRect(0, 291, 467, 679, INKPLATE_BLACK);

    // Weekly forecast (filled black panel; text/icons/arrows drawn in white
    // on top of it).
    inkplate.fillRect(467, 970, 1133, 230, INKPLATE_BLACK);

    // Temperature & precipitation graph area has no outline/fill in the
    // original sketch -- it is drawn directly on the cleared white
    // background by drawTemperaturePrecipGraph().
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

// --- Draw Temperature & Precipitation Graph ---
void Gui::drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    // Layout values for graph placement
    int graphX = 520;
    int graphY = 370;
    int graphWidth = 1040;
    int graphHeight = 590;

    inkplate.setCursor(490, 335);
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(INKPLATE_BLACK);
    inkplate.print("Hourly temperature and precipitation");

    int marginX = 30;
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
    inkplate.setFont(&FreeSans12pt7b);

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
        inkplate.fillRect(x + 20, y + 10, 15, barHeight - 10, INKPLATE_BLUE);

        // Draw precipitation value on top of the bar
        inkplate.setCursor(x + 17, y - 5);
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
    inkplate.setFont(&FreeSansBold24pt7b);
    inkplate.setTextColor(INKPLATE_BLACK);

    inkplate.drawBitmap(40, 40, icon_s_gray, 48, 48, INKPLATE_BLACK);
    inkplate.setCursor(110, 75);
    inkplate.print(userInfo->city);

    inkplate.setFont(&FreeSans18pt7b);
    inkplate.drawBitmap(40, 100, icon_s_thermometer, 48, 48, INKPLATE_BLACK);
    inkplate.setCursor(110, 135);
    inkplate.print(weatherData->currentTemp);
    inkplate.print(userInfo->temperatureLabel);

    inkplate.drawBitmap(40, 160, getWeatherIcon(weatherData->weatherCode), 48, 48, INKPLATE_BLACK);
    inkplate.setCursor(110, 200);
    inkplate.println(weatherData->weatherDescription);

    // Section 2: User Info (white text on the black panel drawn by
    // drawBackground()). No battery reading here - Inkplate 13SPECTRA has
    // no readBattery() implementation in the library (BoardCommon.cpp
    // excludes it for this board), unlike the other Inkplate boards this
    // example was ported from.

    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(INKPLATE_WHITE);

    int yUser = 60;
    inkplate.setCursor(1100, yUser);
    inkplate.println(userInfo->lastUpdatedDate);

    yUser += 60;
    inkplate.setCursor(1100, yUser);
    inkplate.print("Last refresh: ");
    inkplate.println(userInfo->lastUpdatedTime);

    yUser += 60;
    inkplate.setCursor(1100, yUser);
    inkplate.println(userInfo->username);

    // Section 3: Additional Info (feels like, wind, etc.) -- white text on
    // the black panel drawn by drawBackground().
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(INKPLATE_WHITE);

    int y = 375;
    int xPos = 50;

    inkplate.setCursor(xPos, y);
    inkplate.print("Feels like: ");
    inkplate.print(weatherData->feelsLike);
    inkplate.print(userInfo->temperatureLabel);

    y += 105;
    inkplate.setCursor(xPos, y);
    inkplate.print("Sunrise: ");
    inkplate.println(weatherData->sunrise);

    y += 105;
    inkplate.setCursor(xPos, y);
    inkplate.print("Sunset: ");
    inkplate.println(weatherData->sunset);

    y += 105;
    inkplate.setCursor(xPos, y);
    inkplate.print("UV index: ");
    inkplate.println(weatherData->uvIndex);

    y += 105;
    inkplate.setCursor(xPos, y);
    inkplate.print("Wind: ");
    inkplate.print(weatherData->windSpeed);
    inkplate.print(userInfo->speedLabel);

    y += 105;
    inkplate.setCursor(xPos, y);
    inkplate.print("Precipitation: ");
    inkplate.print(weatherData->precipitation);
    inkplate.print(" %");

    // Section 4: Weekly Forecast -- white text/icons/arrows on the black
    // panel drawn by drawBackground().
    inkplate.setTextColor(INKPLATE_WHITE);

    int startX = 530;                      // Starting x-position for the weekly forecast
    int startY = 1015;                     // Starting y-position for the weekly forecast
    int iconSize = 64;                     // Size of the icon
    int margin = 25;                       // Margin between elements
    int dayWidth = iconSize + margin + 63; // Space for icon + margin + text width

    // Loop through the 7-day forecast and display each day
    for (int i = 0; i < 7; i++)
    {
        inkplate.setFont(&FreeSans18pt7b);
        int xPos = startX + i * dayWidth;

        // Day name
        inkplate.setCursor(xPos + 15, startY);
        inkplate.setTextColor(INKPLATE_WHITE);
        inkplate.print(weatherData->dailyNames[i]);

        // Weather icon
        inkplate.setFont(&FreeSans18pt7b);
        inkplate.drawBitmap(xPos + 15, startY + 20, getWeatherIcon(weatherData->dailyWeatherCodes[i]), iconSize,
                            iconSize, INKPLATE_WHITE);
        int tempYStart = startY + 30 + iconSize + margin + 5;

        // === Max Temp - Up Arrow Triangle ===
        int arrowX = xPos;
        int arrowY = tempYStart + 3;
        // Triangle pointing up (white, drawn on the black weekly panel)
        inkplate.fillTriangle(arrowX, arrowY,         // bottom center
                              arrowX - 4, arrowY + 6, // bottom left
                              arrowX + 4, arrowY + 6, // bottom right
                              INKPLATE_WHITE);
        // Max temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 8);
        inkplate.print(weatherData->dailyMaxTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
        // === Min Temp - Down Arrow Triangle ===
        arrowY += 30;
        // Triangle pointing down (white, drawn on the black weekly panel)
        inkplate.fillTriangle(arrowX, arrowY + 6, // top center
                              arrowX - 4, arrowY, // bottom left
                              arrowX + 4, arrowY, // bottom right
                              INKPLATE_WHITE);
        // Min temp text next to it
        inkplate.setCursor(arrowX + 10, arrowY + 14);
        inkplate.print(weatherData->dailyMinTemp[i]);
        inkplate.print(userInfo->temperatureLabel);
    }

    // Section 5: Day or Night indicator (black icon/text on the cleared
    // white background below the additional-info panel).
    int iconX = 100;
    int iconY = 1050;
    if (weatherData->isDay)
    {
        inkplate.drawBitmap(iconX, iconY, icon_s_clear_sky, 48, 48, INKPLATE_BLACK);
        inkplate.setCursor(iconX + 70, iconY + 35);
        inkplate.setTextColor(INKPLATE_BLACK);
        inkplate.setFont(&FreeSansBold24pt7b);
        inkplate.print("Daytime");
    }
    else
    {
        inkplate.drawBitmap(iconX, iconY, icon_s_moon, 48, 48, INKPLATE_BLACK);
        inkplate.setCursor(iconX + 70, iconY + 35);
        inkplate.setTextColor(INKPLATE_BLACK);
        inkplate.setFont(&FreeSansBold24pt7b);
        inkplate.print("Nighttime");
    }

    // Section 6: Graph info
    drawTemperaturePrecipGraph(weatherData, userInfo);

    // Finalize drawing
    inkplate.display();
}
