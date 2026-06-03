/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Basic RTC time and date example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates basic usage of the PCF85063A real-time clock (RTC)
 *              integrated on the Inkplate 6 Flick board. The example shows how
 *              to set the current time and date, read the RTC values, and
 *              display the time on the e-paper screen using partial updates to
 *              minimise flicker.
 *
 *              The display updates once per second and uses partial refresh for
 *              most frames, falling back to a full refresh every 10 partial
 *              updates to maintain image quality.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Set the desired initial time and date in the s_hour / s_minutes /
 *    s_seconds / s_weekday / s_day / s_month / s_year variables below.
 * 2) Build and flash to Inkplate 6 Flick.
 * 3) The RTC is set once on boot, then read and displayed every second.
 *
 * Expected output:
 * - E-paper display shows the current date and time, updating once per second.
 *   Format: HH:MM:SS Weekday, DD/MM/YYYY
 *
 * Notes:
 * - Inkplate 6 Flick uses the PCF85063A RTC chip.
 * - Partial update works only in 1-bit (black & white) mode.
 * - It is not recommended to use partial update on the first refresh after
 *   power-up.
 * - Perform a full refresh every 5-10 partial updates to maintain display
 *   quality.
 * - tm_year stores the full calendar year (e.g. 2026), not years since 1900.
 * - tm_mon is 1-based (1 = January), not 0-based like standard C.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_SIMPLE";

#define REFRESH_DELAY_MS 1000

// Initial time to set on the RTC
static uint8_t s_hour = 12;
static uint8_t s_minutes = 0;
static uint8_t s_seconds = 0;

// Initial date (weekday: 0=Sunday, 1=Monday, ..., 6=Saturday)
static uint8_t s_weekday = 2; // Tuesday
static uint8_t s_day = 3;
static uint8_t s_month = 6;    // 1-based: 6 = June
static uint16_t s_year = 2026; // full calendar year

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

static void printTime(Inkplate &display, uint8_t hour, uint8_t minutes,
                      uint8_t seconds, uint8_t day, uint8_t weekday,
                      uint8_t month, uint16_t year) {
  const char *wday[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};

  print2Digits(display, hour);
  display.print(':');
  print2Digits(display, minutes);
  display.print(':');
  print2Digits(display, seconds);
  display.print(' ');
  display.print(wday[weekday % 7]);
  display.print(", ");
  print2Digits(display, day);
  display.print('/');
  print2Digits(display, month);
  display.print('/');
  display.print(year);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(4);
  display.setTextColor(BLACK, WHITE);

  // Build the tm struct and set RTC time
  // Note: tm_year is the full year (e.g. 2026), tm_mon is 1-based (1=January)
  tm t = {};
  t.tm_hour = s_hour;
  t.tm_min  = s_minutes;
  t.tm_sec  = s_seconds;
  t.tm_wday = s_weekday;
  t.tm_mday = s_day;
  t.tm_mon  = s_month;
  t.tm_year = s_year;

  esp_err_t err = display.rtc.setTime(t);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  int n = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    display.clearDisplay();
    display.setCursor(50, 350);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday, now.tm_mon, now.tm_year);

    if (n > 9) {
      display.display();
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }
  }
}
