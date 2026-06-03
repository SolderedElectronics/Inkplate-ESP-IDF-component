/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates how to use the PCF85063A RTC alarm functionality
 *              on the Inkplate 13SPECTRA board. The example sets the current
 *              time and date, configures an alarm, reads the time periodically,
 *              and displays it on the screen. When the alarm triggers, "ALARM!"
 *              is shown in red on the display.
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
 * 2) Set time, date, and alarm time in the code.
 * 3) "ALARM!" appears on screen when the configured alarm time is reached.
 *
 * Expected output:
 * - Current time displayed and updated every minute.
 * - "ALARM!" printed in red when alarm fires.
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

static const char *TAG = "RTC_ALARM";

#define REFRESH_DELAY_MS 60000

// Set clock
static uint8_t s_hour = 12;
static uint8_t s_minutes = 51;
static uint8_t s_seconds = 10;

// Set date (weekday: 0=Sunday, 1=Monday, ...)
static uint8_t s_weekday = 1;
static uint8_t s_day = 2;
static uint8_t s_month = 2;
static uint16_t s_year = 2026;

// Alarm fires 60 seconds after start
static uint8_t s_alarmHour = 12;
static uint8_t s_alarmMinutes = 52;
static uint8_t s_alarmSeconds = 10;
static uint8_t s_alarmWeekday = 1;
static uint8_t s_alarmDay = 2;

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

  display.rtc.reset();

  tm t = {};
  t.tm_hour = s_hour;
  t.tm_min = s_minutes;
  t.tm_sec = s_seconds;
  t.tm_wday = s_weekday;
  t.tm_mday = s_day;
  t.tm_mon = s_month;
  t.tm_year = s_year;

  esp_err_t err = display.rtc.setTime(t);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  err = display.rtc.setAlarm(s_alarmSeconds, s_alarmMinutes, s_alarmHour,
                             s_alarmDay, s_alarmWeekday);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setAlarm failed: %s", esp_err_to_name(err));

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
    } else {
      display.clearDisplay();
      display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
      display.setCursor(100, 300);
      printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
                now.tm_wday, now.tm_mon, now.tm_year);

      if (display.rtc.checkAlarmFlag()) {
        display.rtc.clearAlarmFlag();
        display.setTextColor(INKPLATE_RED, INKPLATE_WHITE);
        display.setCursor(400, 400);
        display.print("ALARM!");
        ESP_LOGI(TAG, "Alarm triggered!");
      }

      display.display();
    }

    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}
