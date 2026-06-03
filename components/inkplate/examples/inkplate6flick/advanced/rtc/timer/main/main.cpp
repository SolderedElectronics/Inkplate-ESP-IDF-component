/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC countdown timer example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to use the PCF85063A real-time clock (RTC)
 *              hardware timer on the Inkplate 6 Flick board. The RTC timer is
 *              an independent countdown mechanism separate from the time-of-day
 *              clock. It counts down from a configured value at a selectable
 *              clock source frequency and sets a timer flag when it reaches zero.
 *
 *              This example sets a 15-second countdown using the 1 Hz clock
 *              source. The main loop polls the timer flag, clears it, disables
 *              the timer when it fires, and prints "Timer!" on the display.
 *              Remove the disableTimer() call to make the timer repeating.
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
 * 1) Adjust COUNTDOWN_TIME to set the desired countdown duration in seconds.
 * 2) Build and flash to Inkplate 6 Flick.
 * 3) The RTC time and the countdown timer are both configured on boot.
 * 4) Current time is displayed each iteration; "Timer!" appears when the
 *    countdown reaches zero.
 *
 * Expected output:
 * - E-paper display shows the current date and time.
 * - After COUNTDOWN_TIME seconds, "Timer!" appears on the display.
 *
 * Notes:
 * - Inkplate 6 Flick uses the PCF85063A RTC chip.
 * - Available timer clock sources and their ranges:
 *     RTC_TIMER_CLOCK_4096HZ  -> min 244 us,   max ~62 ms   (value: 1-255)
 *     RTC_TIMER_CLOCK_64HZ    -> min 15.6 ms,  max ~3.98 s  (value: 1-255)
 *     RTC_TIMER_CLOCK_1HZ     -> min 1 s,       max 255 s   (value: 1-255)
 *     RTC_TIMER_CLOCK_1PER60HZ-> min 60 s,      max 4 h 15 min
 * - setTimer(clockSource, value, intEnable, intPulse):
 *     intEnable = true  : interrupt output is enabled
 *     intPulse  = true  : interrupt generates a pulse (level otherwise)
 * - To make the timer repeat, remove the disableTimer() call.
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

static const char *TAG = "RTC_TIMER";

#define REFRESH_DELAY_MS 700

// Initial time to set on the RTC
static uint8_t s_hour = 12;
static uint8_t s_minutes = 0;
static uint8_t s_seconds = 0;

// Initial date (weekday: 0=Sunday, 1=Monday, ..., 6=Saturday)
static uint8_t s_weekday = 2; // Tuesday
static uint8_t s_day = 3;
static uint8_t s_month = 6;    // 1-based: 6 = June
static uint16_t s_year = 2026; // full calendar year

// Countdown timer: 15 seconds at 1 Hz
static const uint8_t COUNTDOWN_TIME = 15;

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

  // Set RTC time and date
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

  /*  setTimer(clockSource, value, intEnable, intPulse)
   *
   *  Clock sources:
   *    RTC_TIMER_CLOCK_4096HZ  -> min = 244 us,    max = ~62 ms
   *    RTC_TIMER_CLOCK_64HZ    -> min = 15.625 ms, max = ~3.98 s
   *    RTC_TIMER_CLOCK_1HZ     -> min = 1 s,       max = 255 s
   *    RTC_TIMER_CLOCK_1PER60HZ-> min = 60 s,      max = 4 h 15 min
   *
   *  intEnable: true  = interrupt output enabled
   *  intPulse:  true  = interrupt generates a pulse
   *             false = interrupt level follows timer flag
   */
  err = display.rtc.setTimer(RTC_TIMER_CLOCK_1HZ, COUNTDOWN_TIME, true, false);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTimer failed: %s", esp_err_to_name(err));

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
    display.setCursor(50, 300);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday, now.tm_mon, now.tm_year);

    // Poll timer flag
    if (display.rtc.checkTimerFlag()) {
      display.rtc.clearTimerFlag();
      display.rtc.disableTimer(); // remove this line to make the timer repeat
      display.setCursor(350, 420);
      display.print("Timer!");
      ESP_LOGI(TAG, "RTC timer triggered!");
    }

    if (n > 9) {
      display.display();
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }
  }
}
