/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Basic RTC time and date example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates basic usage of the PCF85063A real-time clock (RTC)
 *              integrated on the Inkplate 13SPECTRA board. The example shows
 *              how to set the current time and date, read the RTC values, and
 *              display the time on the e-paper screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Build and flash to Inkplate 13SPECTRA.
 * 2) Set the initial RTC time and date in the code.
 * 3) The current time is read from the RTC and displayed every minute.
 *
 * Expected output:
 * - Inkplate display shows the current date and time, updated every minute.
 *
 * Notes:
 * - Inkplate 13SPECTRA uses the PCF85063A RTC chip.
 * - Full refresh is used on every update (no partial update on color display).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_SIMPLE";

#define REFRESH_DELAY_MS 60000

// Set clock
static uint8_t s_hour = 13;
static uint8_t s_minutes = 0;
static uint8_t s_seconds = 10;

// Set date (weekday: 0=Sunday, 1=Monday, ...)
static uint8_t s_weekday = 1;
static uint8_t s_day = 2;
static uint8_t s_month = 2;
static uint16_t s_year = 2026;

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

  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);

  tm t = {};
  t.tm_hour = s_hour;
  t.tm_min = s_minutes;
  t.tm_sec = s_seconds;
  t.tm_wday = s_weekday;
  t.tm_mday = s_day;
  t.tm_mon = s_month;
  t.tm_year = s_year;

  display.rtc.reset();
  esp_err_t err = display.rtc.setTime(t);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
    } else {
      display.clearDisplay();
      display.setCursor(80, 300);
      printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
                now.tm_wday, now.tm_mon, now.tm_year);
      display.display();
    }

    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}
