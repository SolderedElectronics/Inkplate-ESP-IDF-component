/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Wake-up button and timer deep sleep example for Soldered
 *              Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to wake the ESP32 from deep sleep on Inkplate
 *              4TEMPERA using an external interrupt (WakeUp button) and a
 *              fallback timer. The example stores a boot counter in RTC memory,
 *              shows the boot count on the e-paper display, and prints the
 *              wake-up reason (button press vs. timer wake-up).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) After displaying boot info, the board enters deep sleep.
 * 3) Wake the board by pressing the WakeUp button, or wait 30 seconds.
 * 4) On each wake, the display updates with the new boot count and wake-up
 *    reason.
 *
 * Expected output:
 * - Inkplate display shows an incrementing boot count.
 * - Wake-up reason is shown as either WakeUp button or timer.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - bootCount is stored in RTC memory (RTC_DATA_ATTR) so it persists across
 *   deep sleep.
 * - WakeUp button wake uses EXT0 wake-up on GPIO36.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TIME_TO_SLEEP_US (30ULL * 1000000ULL)

RTC_DATA_ATTR static int bootCount = 0;

static void displayInfo(Inkplate &display) {
  display.clearDisplay();

  display.setCursor(30, 240);
  display.setTextSize(3);
  display.print("Boot count: ");
  display.print(bootCount);

  display.setCursor(30, 300);
  display.setTextSize(2);

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
  display.setDisplayMode(BLACK_AND_WHITE);

  ++bootCount;

  displayInfo(display);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
  esp_deep_sleep_start();
}
