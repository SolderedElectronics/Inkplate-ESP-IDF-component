/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Wake-up button and timer deep sleep example for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to wake the ESP32 from deep sleep on Inkplate 5
 *              using an external interrupt (WakeUp button on GPIO36) and a
 *              fallback timer. The example stores a boot counter in RTC memory,
 *              shows the boot count on the e-paper display, and prints the
 *              wake-up reason (button press vs. timer wake-up).
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
 * 2) After displaying boot info, the board enters deep sleep.
 * 3) Wake the board by pressing the WakeUp button, or wait 30 seconds.
 * 4) On each wake, the display updates with the new boot count and wake reason.
 *
 * Expected output:
 * - Inkplate display shows an incrementing boot count.
 * - Wake-up reason is shown as either WakeUp button or timer.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - bootCount is stored in RTC memory so it persists across deep sleep.
 * - WakeUp button uses EXT0 wake-up on GPIO36.
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

#include "Inkplate.h"

#define TIME_TO_SLEEP_US (30ULL * 1000000ULL)

RTC_DATA_ATTR static int bootCount = 0;

static void displayInfo(Inkplate &display) {
  display.clearDisplay();
  display.setCursor(30, 40);
  display.setTextSize(3);

  display.print("Boot count: ");
  display.println(bootCount);

  display.setCursor(30, 100);

  switch (esp_sleep_get_wakeup_cause()) {
  case ESP_SLEEP_WAKEUP_EXT0:
    display.println("Wakeup caused by WakeUp button");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    display.println("Wakeup caused by timer");
    break;
  default:
    display.println("Wakeup was not caused by deep sleep");
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
