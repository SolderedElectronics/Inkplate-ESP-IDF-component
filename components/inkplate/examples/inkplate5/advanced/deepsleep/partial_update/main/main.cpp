/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Partial e-paper update with ESP32 deep sleep for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to correctly use partial screen updates together
 *              with ESP32 deep sleep on Inkplate 5. Since partial updates rely
 *              on previously stored screen content in RAM, the screen must be
 *              recreated after waking from deep sleep before calling
 *              partialUpdate(). This example shows how to preserve variables in
 *              RTC memory, rebuild the screen, and safely perform partial updates.
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
 * 2) After first full refresh, the device will enter deep sleep.
 * 3) Every 10 seconds the ESP32 wakes up, updates variables,
 *    rebuilds the screen buffer, and performs a partial update.
 * 4) Observe changing values on the display after each wake cycle.
 *
 * Expected output:
 * - First boot performs a full refresh.
 * - Subsequent wake-ups perform partial updates only.
 * - Counter and decimal value increment after each deep sleep cycle.
 *
 * Notes:
 * - Partial update works only in 1-bit (black & white) mode.
 * - Do NOT use standard partial update examples together with deep sleep.
 * - Always rebuild the screen content after deep sleep before calling
 *   partialUpdate().
 * - It is recommended to perform a full refresh every 5-10 partial updates
 *   to maintain good image quality.
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

#include "esp_attr.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/rtc.h"

#include "Inkplate.h"

#define TIME_TO_SLEEP_US (10ULL * 1000000ULL)

RTC_DATA_ATTR static int counter = 0;
RTC_DATA_ATTR static float decimal = 3.14159265f;

static void createScreen(Inkplate &display) {
  display.setFont(NULL);
  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);

  display.setCursor(220, 300);
  display.print("Inkplate 5 partial update with deep sleep example");
  display.setCursor(490, 350);
  display.print("First variable:");
  display.print(counter);
  display.setCursor(490, 390);
  display.print("Second variable:");
  display.print(decimal, 2);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  createScreen(display);

  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) {
    display.preloadScreen();
    counter++;
    decimal *= 1.23f;
    display.clearDisplay();
    createScreen(display);
    display.partialUpdate(true);
  } else {
    display.display();
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}
