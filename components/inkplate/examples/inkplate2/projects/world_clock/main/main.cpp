/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       World clock example for Soldered Inkplate 2.
 *
 * @details     Draws one analog clock face per configured city/timezone and
 *              keeps them in sync with real time. WiFi is used once, at
 *              boot, to sync the wall clock via SNTP
 *              (display.wifi.setCurrentTime()); from then on, every city's
 *              local time is derived offline from that shared UTC epoch
 *              using a fixed UTC offset (see NetworkFunctions.cpp/.h).
 *
 *              The original Arduino sketch queried a remote REST service
 *              (timeapi.io) to resolve each city name to a full IANA
 *              timezone and fetch its current local time. That network
 *              dependency has been replaced here with local offset math,
 *              which is simpler, does not require a live connection after
 *              the initial NTP sync, and matches the approach already used
 *              by this component's rtc/alarm example.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi connection + Internet access (NTP), at least at boot
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - Cities/timezones: edit the `kCities` array below.
 *
 * How to use:
 * 1) Edit `kCities` in this file with the cities and UTC offsets you want.
 * 2) Configure your WiFi credentials in menuconfig.
 * 3) Build and flash to Inkplate 2.
 * 4) The device connects to WiFi, syncs time via NTP, then draws one
 *    analog clock per configured city and refreshes every minute.
 *
 * Expected output:
 * - One analog clock face per configured city, each with its own label and
 *   an AM/PM indicator, refreshed once a minute.
 *
 * Notes:
 * - UTC offsets are fixed (no automatic daylight-saving adjustment) - update
 *   `kCities` if a city you track changes clocks.
 * - Only two clocks fit side by side on the Inkplate 2's canvas with the
 *   layout used below; add more rows/columns in `drawTime()` if you need
 *   to display more cities at once.
 * - Display mode is color (black/white/red). This sketch uses a full
 *   refresh (display()) every minute.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "NetworkFunctions.h"
#include "SourceSansPro_Regular6pt7b.h"
#include "SourceSansPro_Regular8pt7b.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIFI_CONNECT_TIMEOUT_MS 15000
#define REFRESH_INTERVAL_MS (60 * 1000) // Redraw all clocks once a minute

static const char *TAG = "WORLD_CLOCK";

/**
 * @brief One entry per clock face to draw.
 *
 */
struct CityInfo {
  const char *label;    // Name printed under the clock face
  int utcOffsetMinutes;  // Fixed offset from UTC, in minutes (no DST)
};

// TODO: customize your cities/timezones. Offsets are fixed UTC offsets in
// minutes (no automatic DST) - update twice a year for cities that observe
// daylight saving time. The layout below draws clocks side by side, so keep
// this list to two entries unless you also rework the positions in
// drawTime()/app_main().
static const CityInfo kCities[] = {
    {"Zagreb", 2 * 60},  // CEST, UTC+2 (Croatia, observes DST)
    {"Lima", -5 * 60},   // PET, UTC-5 (Peru, no DST)
};
static const int kNumCities = sizeof(kCities) / sizeof(kCities[0]);

static NetworkFunctions network;

// `display` is NOT a global: it's constructed as a local in app_main() and
// passed by reference into every function that needs it. A file-scope
// `Inkplate display;` would leave its construction order relative to other
// globals unspecified by C++ (cross-translation-unit static init order) —
// harmless on Inkplate2 specifically (no I2C/PCAL peripheral globals to race
// on this board), but the pattern is avoided everywhere in this codebase for
// consistency and because it stops being safe the moment any I2C-backed
// peripheral is added.

/**
 * @brief Draw one analog clock face with an AM/PM indicator and city label.
 *
 * @param x_pos Top-left x coordinate of the clock face.
 * @param y_pos Top-left y coordinate of the clock face.
 * @param hours Local hour (0-23) to render.
 * @param minutes Local minute (0-59) to render.
 * @param city Label printed under the clock face.
 */
static void drawTime(Inkplate &display, uint16_t x_pos, uint16_t y_pos,
                      int hours, int minutes, const char *city) {
  const bool pm = hours >= 12;
  const uint16_t w = 80;

  float xStart[12], yStart[12], xEnd[12], yEnd[12];

  display.drawCircle(x_pos + w / 2, y_pos + w / 2, w / 2, INKPLATE2_BLACK);
  display.drawCircle(x_pos + w / 2, y_pos + w / 2, w / 2 + 1, INKPLATE2_BLACK);

  display.drawThickLine(x_pos + w / 2, y_pos, x_pos + w / 2, y_pos + 5,
                        INKPLATE2_BLACK, 2);
  display.drawThickLine(x_pos + w, y_pos + w / 2, x_pos + w - 5, y_pos + w / 2,
                        INKPLATE2_BLACK, 2);
  display.drawThickLine(x_pos + w / 2, y_pos + w, x_pos + w / 2, y_pos + w - 5,
                        INKPLATE2_BLACK, 2);
  display.drawThickLine(x_pos, y_pos + w / 2, x_pos + 5, y_pos + w / 2,
                        INKPLATE2_BLACK, 2);

  for (int i = 0; i < 12; i++) {
    float angle_rad = 30.0f * i * (float)M_PI / 180.0f;

    xStart[i] = x_pos + (cosf(angle_rad) * w / 2) + w / 2;
    yStart[i] = y_pos + (sinf(angle_rad) * w / 2) + w / 2;

    xEnd[i] = x_pos + (cosf(angle_rad) * (w * 0.85f) / 2) + w / 2;
    yEnd[i] = y_pos + (sinf(angle_rad) * (w * 0.85f) / 2) + w / 2;

    display.drawThickLine(xStart[i], yStart[i], xEnd[i], yEnd[i],
                          INKPLATE2_BLACK, 1);
  }

  int x_minute =
      x_pos + w / 2 + 30 * sinf((minutes / 60.0f) * 2.0f * (float)M_PI);
  int y_minute =
      y_pos + w / 2 - 30 * cosf((minutes / 60.0f) * 2.0f * (float)M_PI);

  float h = (hours % 12) / 12.0f + minutes / 720.0f;
  int x_hour = x_pos + w / 2 + 22 * sinf(h * 2.0f * (float)M_PI);
  int y_hour = y_pos + w / 2 - 22 * cosf(h * 2.0f * (float)M_PI);

  display.drawThickLine(x_pos + w / 2, y_pos + w / 2, x_minute, y_minute,
                        INKPLATE2_RED, 2);
  display.drawThickLine(x_pos + w / 2, y_pos + w / 2, x_hour, y_hour,
                        INKPLATE2_BLACK, 3);

  display.fillCircle(x_pos + w / 2, y_pos + w / 2, 5, INKPLATE2_BLACK);

  display.setTextSize(1);
  display.setFont(&SourceSansPro_Regular8pt7b);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);

  display.setCursor(x_pos + 40 - (int)strlen(city) * 5, 100);
  for (size_t cnt = 0; city[cnt] != '\0'; cnt++)
    display.print(city[cnt] == '_' ? ' ' : city[cnt]);

  display.setCursor(x_pos + 32, y_pos + 62);
  display.setFont(&SourceSansPro_Regular6pt7b);
  display.print(pm ? "PM" : "AM");
}

/**
 * @brief Redraw every configured city's clock face for the current time.
 *
 */
static void drawAllClocks(Inkplate &display) {
  display.clearDisplay(); // Start from a clean buffer every refresh

  for (int i = 0; i < kNumCities; i++) {
    int hours = 0, minutes = 0;
    if (!network.getData(kCities[i].utcOffsetMinutes, &hours, &minutes)) {
      ESP_LOGE(TAG, "Failed to compute time for %s", kCities[i].label);
      continue;
    }

    ESP_LOGI(TAG, "%s time %02d:%02d", kCities[i].label, hours, minutes);

    // Two clocks side by side, matching the original Arduino layout.
    const uint16_t x_pos = 17 + i * 98;
    drawTime(display, x_pos, 1, hours, minutes, kCities[i].label);
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  // --- DISPLAY SELF-TEST (so we know the panel can refresh) ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);
  display.setCursor(5, 15);
  display.print("DISPLAY OK - starting...");
  display.drawRect(0, 0, display.width(), display.height(), INKPLATE2_BLACK);
  display.display();

  // --- WIFI + NTP (menuconfig-driven credentials, never hardcoded) ---
  if (display.wifi.begin() != ESP_OK ||
      !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
    ESP_LOGE(TAG, "WiFi connection failed");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Unable to connect to WiFi.\nPlease check SSID and PASS!");
    display.display();
    return;
  }

  display.wifi.setCurrentTime();
  network.begin();

  ESP_LOGI(TAG, "WiFi connected, time synced. Starting clock refresh loop.");

  while (true) {
    drawAllClocks(display);
    vTaskDelay(pdMS_TO_TICKS(REFRESH_INTERVAL_MS));
  }
}
