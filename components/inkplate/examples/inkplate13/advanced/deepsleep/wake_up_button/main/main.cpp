/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Deep sleep with button or timer wakeup for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates two wakeup sources for ESP32-S3 deep sleep: a
 *              2-minute timer and the hardware wake-up button (GPIO 18). On
 *              each wakeup the boot count (stored in RTC memory) is incremented
 *              and the wakeup cause is printed on the display. After updating
 *              the screen the board re-enters deep sleep.
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
 * 2) The board shows the boot count and wakeup reason, then sleeps for 2 minutes.
 * 3) Press the wake-up button to wake the board early.
 *
 * Expected output:
 * - Boot count and wakeup reason displayed on the e-paper screen.
 *
 * Notes:
 * - RTC_DATA_ATTR keeps the boot counter in RTC memory across deep sleep cycles.
 * - The wake-up button is connected to GPIO 18.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TIME_TO_SLEEP_US (120ULL * 1000000ULL)
#define WAKEUP_GPIO      GPIO_NUM_18

RTC_DATA_ATTR static int bootCount = 0;

static void displayInfo(Inkplate &display) {
  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(2);
  display.setCursor(10, 180);

  display.print("Boot count: ");
  display.print(bootCount);

  display.setCursor(10, 220);

  switch (esp_sleep_get_wakeup_cause()) {
  case ESP_SLEEP_WAKEUP_EXT0:
    display.print("Wakeup caused by WakeUp button");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    display.print("Wakeup caused by timer");
    break;
  default:
    display.print("Wakeup was not caused by deep sleep");
    break;
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  ++bootCount;

  displayInfo(display);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);
  esp_deep_sleep_start();
}
