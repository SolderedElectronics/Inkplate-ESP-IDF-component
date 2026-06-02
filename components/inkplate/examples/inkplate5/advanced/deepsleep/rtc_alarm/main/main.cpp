/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm wake-up with deep sleep for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to use the onboard RTC alarm interrupt to wake
 *              the Inkplate 5 from ESP32 deep sleep. The RTC alarm is
 *              configured to trigger periodically, waking the board, refreshing
 *              the e-paper display with the current date and time, and then
 *              returning the system back to low-power deep sleep mode.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Build and flash to Inkplate 5.
 * 2) On first boot, the RTC time and date are initialized if not already set.
 * 3) The current time and date are shown on the display.
 * 4) The board enters deep sleep and wakes up every 10 seconds via the RTC alarm.
 * 5) After each wake-up, the display is refreshed and the board goes back to sleep.
 *
 * Expected output:
 * - Inkplate display shows the current weekday, date, and time.
 * - Display refreshes automatically on every RTC alarm wake-up.
 *
 * Notes:
 * - RTC alarm interrupt is connected to GPIO39 on Inkplate 5.
 * - When using deep sleep, all application logic must be placed in app_main().
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE5
#error \
    "Wrong board selection for this example, please select Inkplate5 in the boards menu."
#endif

#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

#include "Inkplate.h"

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d, 10);
}

static void printCurrentTime(Inkplate &display) {
  display.setCursor(385, 360);
  display.setTextSize(3);

  tm t = {};
  display.rtc.getTime(&t);

  switch (t.tm_wday) {
  case 0: display.print("Sunday, ");    break;
  case 1: display.print("Monday, ");    break;
  case 2: display.print("Tuesday, ");   break;
  case 3: display.print("Wednesday, "); break;
  case 4: display.print("Thursday, ");  break;
  case 5: display.print("Friday, ");    break;
  case 6: display.print("Saturday, ");  break;
  }

  display.print(t.tm_mday);
  display.print(".");
  display.print(t.tm_mon);
  display.print(".");
  display.print(t.tm_year);
  display.print(". ");

  display.setCursor(490, 400);
  print2Digits(display, t.tm_hour);
  display.print(':');
  print2Digits(display, t.tm_min);
  display.print(':');
  print2Digits(display, t.tm_sec);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.rtc.clearAlarmFlag();

  if (!display.rtc.isSet()) {
    tm t = {};
    t.tm_hour = 13;
    t.tm_min  = 30;
    t.tm_sec  = 0;
    t.tm_mday = 5;
    t.tm_mon  = 12;
    t.tm_year = 2022;
    t.tm_wday = 1;
    display.rtc.setTime(t);
  }

  display.clearDisplay();
  printCurrentTime(display);
  display.display();

  time_t now;
  display.rtc.getTime(&now);
  display.rtc.setAlarmEpoch(now + 10);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);
  esp_deep_sleep_start();
}
