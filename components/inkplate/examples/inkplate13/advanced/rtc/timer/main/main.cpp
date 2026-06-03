/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC timer functionality example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates how to use the PCF85063A RTC timer on the
 *              Inkplate 13SPECTRA board. The example sets the time and date,
 *              configures a 30-second countdown timer, reads current time
 *              periodically, and prints "Timer!" when the timer event fires.
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
 * 2) The 30-second timer starts automatically.
 * 3) "Timer!" appears on screen after 30 seconds.
 *
 * Expected output:
 * - Current time displayed and updated every minute.
 * - "Timer!" shown in red after the countdown expires.
 *
 * Notes:
 * - Timer is configured as one-shot (disableTimer() called after first event).
 * - Remove the disableTimer() call to make the timer repeating.
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

static const char *TAG = "RTC_TIMER";

#define REFRESH_DELAY_MS   60000
#define COUNTDOWN_SECONDS  30

// Set clock
static uint8_t s_hour = 12;
static uint8_t s_minutes = 50;
static uint8_t s_seconds = 30;

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
  display.display();
  display.setTextSize(5);
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

  /*  RTC_TIMER_CLOCK_4096HZ  -> min = 244us,    max = 62.256ms
   *  RTC_TIMER_CLOCK_64HZ    -> min = 15.625ms, max = 3.984s
   *  RTC_TIMER_CLOCK_1HZ     -> min = 1s,       max = 255s
   *  RTC_TIMER_CLOCK_1PER60HZ-> min = 60s,      max = 4h15min
   */
  err = display.rtc.setTimer(RTC_TIMER_CLOCK_1HZ, COUNTDOWN_SECONDS, true, false);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTimer failed: %s", esp_err_to_name(err));

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
    } else {
      display.clearDisplay();
      display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
      display.setCursor(60, 300);
      printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
                now.tm_wday, now.tm_mon, now.tm_year);

      if (display.rtc.checkTimerFlag()) {
        display.rtc.clearTimerFlag();
        display.rtc.disableTimer(); // remove to make timer repeating
        display.setTextColor(INKPLATE_RED, INKPLATE_WHITE);
        display.setCursor(400, 400);
        display.print("Timer!");
        ESP_LOGI(TAG, "Timer triggered!");
      }

      display.display();
    }

    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}
