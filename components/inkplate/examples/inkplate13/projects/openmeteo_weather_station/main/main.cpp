/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Open-Meteo weather station dashboard in 6-color e-paper with
 *              periodic deep-sleep updates on Soldered Inkplate 13SPECTRA.
 *
 * @details     Ported from the Inkplate-Arduino-library's
 *              Inkplate13SPECTRA_OpenMeteo_Weather_Station example. Connects
 *              Inkplate 13SPECTRA to WiFi, synchronizes local time via NTP
 *              (using the configured UTC offset), fetches current/daily/
 *              hourly weather data from the free Open-Meteo API (no API key
 *              required) for the configured latitude/longitude, and renders
 *              a dashboard-style screen using a small GUI helper layer
 *              (Gui / WeatherData / NetworkFunctions).
 *
 *              After drawing the UI, the ESP32 enters deep sleep for
 *              REFRESH_INTERVAL_US (default: 30 minutes, matching the
 *              original sketch's TIME_TO_SLEEP). On wake-up, the ESP32
 *              restarts and app_main() runs again from scratch, fetching
 *              fresh weather data and refreshing the screen. The dashboard
 *              also shows a username/city label, a day/night indicator, and
 *              the battery voltage.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable (battery recommended for deployment)
 * - Extra:      WiFi (2.4 GHz) with internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your SSID/password
 * - Set LATITUDE / LONGITUDE / UTC_OFFSET_HOURS below for your location.
 * - Optional: set MY_USERNAME / MY_CITY (display only) and USE_METRIC_UNITS.
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Set LATITUDE, LONGITUDE and UTC_OFFSET_HOURS below for your location.
 * 3) Build and flash to Inkplate 13SPECTRA.
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
 * - Inkplate 13SPECTRA is a 6-color e-paper panel (1600x1200 landscape, the
 *   board's default working orientation -- see Gui.cpp for why this differs
 *   from the raw E_INK_WIDTH/E_INK_HEIGHT panel constants); there is no
 *   grayscale mode and no setDisplayMode() call on this board -- colors are
 *   drawn directly with the INKPLATE_BLACK/WHITE/YELLOW/RED/BLUE/GREEN
 *   macros. Unlike Inkplate 6Color, there is no INKPLATE_ORANGE on this
 *   board. See Gui.cpp for exactly which colors are used where (black text/
 *   icons on white, black status/info/weekly panels with white text/icons,
 *   and a red "hot" / blue "cold" accent for temperature extremes in the
 *   hourly graph).
 * - Partial update is not available on this board either, so the UI is
 *   always fully refreshed.
 * - Deep sleep restarts the ESP32; all runtime state is lost and
 *   recomputed on every wake cycle.
 * - Latitude/longitude/UTC offset are compile-time #defines below; there is
 *   no runtime location picker.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
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
// TODO: fill in your UTC offset in hours (e.g. 1 for UTC+1, -4 for UTC-4).
#define UTC_OFFSET_HOURS 1
// Set to false to use Imperial units (Fahrenheit / mph) instead of metric.
#define USE_METRIC_UNITS true

// --- Display-only user/city labels ---
// TODO: fill in the name to show on screen.
#define MY_USERNAME "Soldered"
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
    Inkplate display; // No setDisplayMode(): Inkplate 13SPECTRA has a single 6-color mode.
    display.clearDisplay(); // Clear the frame buffer.

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

        // Gather city/user info. No battery reading on this board - see
        // Gui.h.
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
