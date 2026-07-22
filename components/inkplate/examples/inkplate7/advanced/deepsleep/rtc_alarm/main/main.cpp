/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm wake-up with deep sleep for Soldered Inkplate 7.
 *
 * @details     Demonstrates how to use the onboard PCF85063A RTC alarm
 *              interrupt to wake the Inkplate 7 from ESP32-S3 deep
 *              sleep. On each wake-up the current date and time are read from
 *              the RTC, displayed on the e-paper screen, a new alarm is set
 *              60 seconds in the future, and the board re-enters deep sleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Build and flash to Inkplate 7.
 * 2) On first boot the RTC is initialized with the time set in this file.
 * 3) The current time is displayed, then the board sleeps until the RTC alarm.
 * 4) Board wakes every 60 seconds, refreshes the display, and sleeps again.
 *
 * Expected output:
 * - Inkplate display shows the current weekday, date, and time.
 * - Display refreshes automatically on every RTC alarm wake-up.
 *
 * Notes:
 * - RTC alarm interrupt is connected to GPIO 18 on Inkplate 7.
 * - All application logic runs in app_main; deep sleep re-enters from there.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE7
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate7 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_DEEPSLEEP";

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

static void printCurrentTime(Inkplate &display) {
  display.setCursor(50, 250);
  display.setTextSize(4);
  display.setTextColor(INKPLATE_BLUE, INKPLATE_WHITE);

  tm t = {};
  display.rtc.getTime(&t);

  const char *wday[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};
  display.print(wday[t.tm_wday % 7]);
  display.print(", ");
  display.print(t.tm_mday);
  display.print(".");
  display.print(t.tm_mon);
  display.print(".");
  display.print(t.tm_year);
  display.print(". ");

  display.setCursor(50, 400);
  print2Digits(display, t.tm_hour);
  display.print(':');
  print2Digits(display, t.tm_min);
  display.print(':');
  print2Digits(display, t.tm_sec);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.rtc.clearAlarmFlag();

  // Initialize RTC only on first boot (when not yet set)
  if (!display.rtc.isSet()) {
    tm t = {};
    t.tm_hour = 13;
    t.tm_min = 30;
    t.tm_sec = 0;
    t.tm_mday = 5;
    t.tm_mon = 12;
    t.tm_year = 2022;
    t.tm_wday = 1;
    display.rtc.setTime(t);
    ESP_LOGI(TAG, "RTC initialized");
  }

  printCurrentTime(display);
  display.display();

  // Set alarm 60 seconds from now
  time_t now;
  display.rtc.getTime(&now);
  display.rtc.setAlarmEpoch(now + 60);

  // RTC interrupt is on GPIO 18 on Inkplate 7
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_18, 0);
  esp_deep_sleep_start();
}
