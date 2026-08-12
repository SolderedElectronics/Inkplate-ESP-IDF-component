/**
 **************************************************
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Open-Meteo weather station: fetch current conditions/forecast
 *              over WiFi, render a dashboard, then deep sleep and refresh
 *              periodically.
 *
 * @details     This example demonstrates a simple "weather station" dashboard
 *              on Inkplate 2 using the free Open-Meteo API (no API key
 *              needed). On each boot, the board connects to WiFi, requests
 *              weather data for the configured latitude/longitude via a
 *              NetworkFunctions helper (esp_http_client + cJSON), and hands
 *              the result to a Gui class for rendering.
 *
 *              To add variety, the example alternates between two different
 *              dashboard layouts on each wake cycle using an RTC-persisted
 *              counter (bootCount, stored in RTC memory so it survives deep
 *              sleep). After drawing the screen, the ESP32 enters deep sleep
 *              and wakes every REFRESH_INTERVAL_US to refresh the data.
 *
 *              Deep sleep resets the ESP32 each time it wakes up, so
 *              execution always restarts from app_main(). The display
 *              retains the last image while the device is asleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      Stable WiFi connection + Internet access (Open-Meteo, no key
 *               required)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - main.cpp   -> LOCATION_LAT / LOCATION_LON: coordinates for your location
 * - main.cpp   -> MY_CITY / MY_USERNAME: display labels
 * - main.cpp   -> METRIC_UNITS: true for Celsius/km-h, false for
 *               Fahrenheit/mph
 *
 * How to use:
 * 1) Set LOCATION_LAT / LOCATION_LON to your location's coordinates.
 * 2) Optionally set MY_CITY and MY_USERNAME for the display labels.
 * 3) Choose units with METRIC_UNITS (true/false).
 * 4) Configure WiFi credentials via menuconfig.
 * 5) Build and flash to Inkplate 2.
 * 6) The device fetches weather, displays the dashboard, then deep sleeps and
 *    refreshes automatically every REFRESH_INTERVAL_US.
 *
 * Expected output:
 * - Display: a weather dashboard; the layout alternates between two designs
 *   on each wake (odd/even bootCount).
 * - On WiFi failure: a WiFi error screen, then a short deep sleep and retry.
 * - On API failure: an API error screen.
 *
 * Notes:
 * - Display mode is 1-bit + red (Inkplate 2 tri-color palette). Full refresh
 *   is used for dashboard rendering.
 * - Deep sleep restarts the ESP32; all logic lives in app_main(), there is no
 *   loop().
 * - bootCount is stored in RTC memory (RTC_DATA_ATTR), which survives deep
 *   sleep but resets on power loss or a fresh flash.
 * - Unlike the original Arduino sketch (which derived "now" from
 *   configTime()/NTP using a user-supplied UTC offset), this port reads
 *   "now" directly from the Open-Meteo response itself (current.time /
 *   daily.time[0], requested with "timezone=auto") - see Network.cpp for
 *   details. This means no timezone/NTP configuration is needed here at all.
 * - Weather accuracy and availability depend on Open-Meteo and network
 *   access.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 *
 * @date        2026
 * @license     GNU GPL V3
 **************************************************/

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "includes.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OPENMETEO_WEATHER";

// --- Location (required by the Open-Meteo API) ---
// TODO: fill in the latitude/longitude of the location to show weather for.
#define LOCATION_LAT 45.5550f
#define LOCATION_LON 18.6955f

// --- Display labels ---
// TODO: fill in your city/username for the dashboard labels.
#define MY_CITY "Osijek"
#define MY_USERNAME "Username"

// true = Celsius/km-h, false = Fahrenheit/mph
#define METRIC_UNITS true

// --- Deep sleep configuration ---
#define REFRESH_INTERVAL_US (600ULL * 1000000ULL)   // 10 minutes between refreshes
#define WIFI_RETRY_INTERVAL_US (30ULL * 1000000ULL) // 30 seconds before retrying WiFi

// This variable persists across deep sleep resets (stored in RTC memory) and
// is used to alternate between the two dashboard layouts on each wake.
RTC_DATA_ATTR static int bootCount = 0;

extern "C" void app_main(void)
{
    Inkplate display;
    NetworkFunctions network;
    NetworkFunctions::UserInfo userInfo = {};
    WeatherData weatherData;
    Gui gui(display);

    display.clearDisplay();

    // Connect to WiFi using credentials configured via menuconfig.
    display.wifi.begin();
    if (!display.wifi.waitForConnect(30000))
    {
        ESP_LOGE(TAG, "Unable to connect to WiFi");
        gui.wifiError();

        esp_sleep_enable_timer_wakeup(WIFI_RETRY_INTERVAL_US);
        esp_deep_sleep_start();
        return; // Never reached, app_main() restarts on wake.
    }

    ESP_LOGI(TAG, "WiFi connected, fetching weather data...");

    // Gather user info/preferences
    snprintf(userInfo.city, sizeof(userInfo.city), "%s", MY_CITY);
    snprintf(userInfo.username, sizeof(userInfo.username), "%s", MY_USERNAME);
    userInfo.useMetric = METRIC_UNITS;

    // Fetch weather data for the configured coordinates
    network.fetchWeatherData(&weatherData, &userInfo, LOCATION_LAT, LOCATION_LON);

    if (userInfo.apiError)
    {
        ESP_LOGE(TAG, "Weather fetch failed");
        gui.apiError();
    }
    else
    {
        if (bootCount % 2 == 1)
            gui.displayWeatherData(&weatherData, &userInfo); // Odd boots: 3-day forecast strip
        else
            gui.displayWeatherData2(&weatherData, &userInfo); // Even boots: current-conditions summary

        bootCount++;
    }

    ESP_LOGI(TAG, "Dashboard updated, entering deep sleep");

    // Sleep to save power; wakes every REFRESH_INTERVAL_US to refresh
    esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
    esp_deep_sleep_start();
}
