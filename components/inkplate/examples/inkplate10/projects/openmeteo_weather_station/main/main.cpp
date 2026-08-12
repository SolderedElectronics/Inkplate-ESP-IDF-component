/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Open-Meteo weather station dashboard in 3-bit grayscale with
 *              periodic deep-sleep updates on Soldered Inkplate 10.
 *
 * @details     Ported from the Inkplate-Arduino-library's
 *              Inkplate10_OpenMeteo_Weather_Station example. Connects
 *              Inkplate 10 to WiFi, synchronizes local time via NTP (using
 *              the configured UTC offset), fetches current/daily/hourly
 *              weather data from the free Open-Meteo API (no API key
 *              required) for the configured latitude/longitude, and renders
 *              a dashboard-style screen using a small GUI helper layer
 *              (Gui / WeatherData / NetworkFunctions).
 *
 *              After drawing the UI, the ESP32 enters deep sleep for
 *              REFRESH_INTERVAL_US (default: 30 minutes, matching the
 *              original sketch's TIME_TO_SLEEP). On wake-up, the ESP32
 *              restarts and app_main() runs again from scratch, fetching
 *              fresh weather data and refreshing the screen. The dashboard
 *              also shows a username/city label and the battery voltage.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable (battery recommended for deployment)
 * - Extra:      WiFi (2.4 GHz) with internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your SSID/password
 * - Set LATITUDE / LONGITUDE / UTC_OFFSET_HOURS below for your location.
 * - Optional: set MY_USERNAME / MY_CITY (display only) and USE_METRIC_UNITS.
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Set LATITUDE, LONGITUDE and UTC_OFFSET_HOURS below for your location.
 * 3) Build and flash to Inkplate 10.
 * 4) The device connects to WiFi, fetches weather data, draws the
 *    dashboard, then deep sleeps. It wakes and refreshes automatically
 *    every REFRESH_INTERVAL_US.
 *
 * Expected output:
 * - E-paper: weather dashboard (current conditions, 7-day forecast, hourly
 *   temperature/precipitation graph, day/night indicator), plus a city/user
 *   label and battery voltage.
 * - Error screens: a WiFi error screen if the connection fails; an API
 *   error screen if the Open-Meteo fetch/parsing fails.
 * - Serial Monitor: WiFi/time-sync status and HTTP/API error messages.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (8 levels); partial update is not
 *   available in grayscale mode, so the UI is always fully refreshed.
 * - Deep sleep restarts the ESP32; all runtime state is lost and
 *   recomputed on every wake cycle.
 * - Latitude/longitude/UTC offset are compile-time #defines below; there is
 *   no runtime location picker.
 * - Orientation: this example does NOT call setRotation(). The original
 *   sketch never rotates the display either, and Gui.cpp's layout
 *   coordinates (e.g. fillRect calls reaching x=1175, y=795) confirm it
 *   targets the native 1200x825 landscape canvas, not a rotated 825x1200
 *   portrait one -- rotating would clip the majority of the dashboard's
 *   width instead. Unlike this board's smaller Inkplate 6 sibling port
 *   (whose upstream sketch reuses the same fixed panel coordinates and
 *   therefore clips its day/night indicator box off the bottom of the
 *   smaller 800x600 canvas), the upstream Inkplate 10 sketch's Gui.cpp
 *   already targets this panel's full 1200x825 resolution -- every layout
 *   coordinate (including the day/night box at y=645-795) fits within the
 *   visible canvas, so Gui.cpp is ported unchanged instead of being
 *   rescaled.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "esp_log.h"
#include "esp_sleep.h"
#include "includes.h"
#include <cstdio>

static const char *TAG = "OPENMETEO";

// --- Location / units configuration ---
// TODO: fill in your location's latitude.
#define LATITUDE 45.5550f
// TODO: fill in your location's longitude.
#define LONGITUDE 18.6955f
// TODO: fill in your UTC offset in hours (e.g. 2 for UTC+2, -4 for UTC-4).
#define UTC_OFFSET_HOURS 2
// Set to false to use Imperial units (Fahrenheit / mph) instead of metric.
#define USE_METRIC_UNITS true

// --- Display-only user/city labels ---
// TODO: fill in the name to show on screen.
#define MY_USERNAME "Username"
// TODO: fill in the city name to show on screen.
#define MY_CITY "Osijek"

// NTP server used to synchronize local time.
#define NTP_SERVER "pool.ntp.org"

// Max time to wait for a WiFi connection, in milliseconds.
#define WIFI_CONNECT_TIMEOUT_MS 30000

// Deep sleep interval between weather refreshes, in microseconds. Matches
// the original sketch's TIME_TO_SLEEP (1800 s = 30 min), used for both the
// success and the WiFi/API-error paths, exactly like the original setup().
#define REFRESH_INTERVAL_US (1800ULL * 1000000ULL)

extern "C" void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE); // 3-bit grayscale, matching the original sketch's INKPLATE_3BIT mode.
    display.clearDisplay();            // Clear the frame buffer.

    NetworkFunctions network;
    NetworkFunctions::UserInfo userInfo = {};
    WeatherData weatherData = {};
    Gui gui(display);

    ESP_LOGI(TAG, "Connecting to WiFi...");
    if (display.wifi.begin() != ESP_OK || !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS))
    {
        ESP_LOGE(TAG, "WiFi connection failed");
        gui.wifiError();
    }
    else
    {
        ESP_LOGI(TAG, "WiFi connected, synchronizing time...");
        network.setupTime(UTC_OFFSET_HOURS, NTP_SERVER);

        // Gather battery and city/user info.
        gui.voltage = display.readBattery();
        snprintf(userInfo.city, sizeof(userInfo.city), "%s", MY_CITY);
        snprintf(userInfo.username, sizeof(userInfo.username), "%s", MY_USERNAME);
        userInfo.useMetric = USE_METRIC_UNITS;

        ESP_LOGI(TAG, "Fetching weather data for (%f, %f)...", (double)LATITUDE, (double)LONGITUDE);
        network.fetchWeatherData(&weatherData, &userInfo, LATITUDE, LONGITUDE);

        if (userInfo.apiError)
        {
            ESP_LOGE(TAG, "Open-Meteo fetch/parse failed");
            gui.apiError();
        }
        else
        {
            gui.drawBackground();
            gui.displayWeatherData(&weatherData, &userInfo);
        }
    }

    // Sleep to save power; wakes every REFRESH_INTERVAL_US and restarts from
    // app_main() to refresh the dashboard, matching the original sketch's
    // single, unconditional deep-sleep call at the end of setup().
    ESP_LOGI(TAG, "Entering deep sleep for %.0f seconds", (double)REFRESH_INTERVAL_US / 1000000.0);
    esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
    esp_deep_sleep_start();
}
