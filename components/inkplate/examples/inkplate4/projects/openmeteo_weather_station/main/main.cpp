/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Open-Meteo weather station dashboard in 3-bit grayscale with
 *              periodic deep-sleep updates on Soldered Inkplate 4TEMPERA.
 *
 * @details     Ported from the Inkplate-Arduino-library's
 *              Inkplate4TEMPERA_OpenMeteo_Weather_Station example. Connects
 *              Inkplate 4TEMPERA to WiFi, synchronizes local time via NTP
 *              (using the configured UTC offset), fetches current/daily/
 *              hourly weather data from the free Open-Meteo API (no API key
 *              required) for the configured latitude/longitude, and renders
 *              a dashboard-style screen using a small GUI helper layer
 *              (Gui / WeatherData / NetworkFunctions).
 *
 *              After drawing the UI, the ESP32 enters deep sleep for
 *              REFRESH_INTERVAL_US (default: 30 minutes). On wake-up, the
 *              ESP32 restarts and app_main() runs again from scratch,
 *              fetching fresh weather data and refreshing the screen. The
 *              dashboard also shows a username/city label and the battery
 *              voltage.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB-C cable
 * - Extra:      WiFi (2.4 GHz) with internet access
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 * - Menuconfig -> WiFi Configuration -> Enter your SSID/password
 * - Set LATITUDE / LONGITUDE / UTC_OFFSET_HOURS below for your location.
 * - Optional: set MY_USERNAME / MY_CITY (display only) and USE_METRIC_UNITS.
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Set LATITUDE, LONGITUDE and UTC_OFFSET_HOURS below for your location.
 * 3) Build and flash to Inkplate 4TEMPERA.
 * 4) The device connects to WiFi, fetches weather data, draws the
 *    dashboard, then deep sleeps. It wakes and refreshes automatically
 *    every REFRESH_INTERVAL_US.
 *
 * Expected output:
 * - E-paper: weather dashboard (current conditions, 5-day forecast, hourly
 *   temperature/precipitation graph), plus a city/user label and battery
 *   voltage.
 * - Error screens: a WiFi error screen if the connection fails; an API
 *   error screen if the Open-Meteo fetch/parsing fails.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (8 levels); partial update is not
 *   available in grayscale mode, so the UI is always fully refreshed.
 * - Deep sleep restarts the ESP32; all runtime state is lost and
 *   recomputed on every wake cycle.
 * - Latitude/longitude/UTC offset are compile-time #defines below; there is
 *   no runtime location picker.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Gui.h"
#include "Inkplate.h"
#include "Network.h"
#include "WeatherData.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

// Deep sleep interval between weather refreshes, in microseconds (30 min).
#define REFRESH_INTERVAL_US (1800ULL * 1000000ULL)
// Deep sleep interval before retrying after a WiFi connection failure.
#define WIFI_RETRY_INTERVAL_US (60ULL * 1000000ULL)
// Max time to wait for a WiFi connection, in milliseconds.
#define WIFI_CONNECT_TIMEOUT_MS 30000

extern "C" void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE);
    display.clearDisplay();

    NetworkFunctions network;
    NetworkFunctions::UserInfo userInfo = {};
    WeatherData weatherData = {};
    Gui gui(display);

    ESP_LOGI(TAG, "Connecting to WiFi...");
    display.wifi.begin();
    bool connected = display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS);

    if (!connected)
    {
        ESP_LOGE(TAG, "WiFi connection failed");
        gui.wifiError();

        // Go back to sleep for a short while, then retry.
        display.frontlight.setState(false);
        display.touchscreen.shutdown();
        esp_sleep_enable_timer_wakeup(WIFI_RETRY_INTERVAL_US);
        esp_deep_sleep_start();
        return; // Never reached; app_main() restarts on wake.
    }

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

    ESP_LOGI(TAG, "Entering deep sleep for %.0f seconds", (double)REFRESH_INTERVAL_US / 1000000.0);

    // Save power while sleeping; wakes every REFRESH_INTERVAL_US.
    display.frontlight.setState(false);
    display.touchscreen.shutdown();
    esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);
    esp_deep_sleep_start();
}
