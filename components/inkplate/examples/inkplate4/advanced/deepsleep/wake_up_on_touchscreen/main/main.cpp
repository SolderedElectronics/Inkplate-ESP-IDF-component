/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Wake-up from deep sleep on button press example for Soldered
 *              Inkplate 4TEMPERA.
 *
 * @details     Demonstrates low-power wake-up behavior on Inkplate 4TEMPERA
 *              using ESP32 deep sleep and external wake sources. The
 *              touchscreen controller is initialized before sleep, and both the
 *              WakeUp button (GPIO36) and a fallback timer are configured as
 *              wake sources. On each boot, a counter stored in RTC memory
 *              (RTC_DATA_ATTR) is incremented and the display shows the boot
 *              count and wake-up cause, then the system enters deep sleep.
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
 * 2) The screen shows the current boot count and the wake-up reason.
 * 3) Wait for the timer wake-up, or press the WakeUp button to wake the device.
 * 4) Observe the boot count increment and the reported wake-up cause.
 *
 * Expected output:
 * - Display shows boot count and wakeup cause (button / timer / other).
 *
 * Notes:
 * - Deep sleep resets the MCU; RTC_DATA_ATTR variables (bootCount) persist.
 * - Wake sources: EXT0 on GPIO36 (WakeUp button) and a 30-second timer.
 * - Frontlight is disabled before sleep to save power.
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

  display.setCursor(30, 310);
  display.setTextSize(2);

  switch (esp_sleep_get_wakeup_cause()) {
  case ESP_SLEEP_WAKEUP_EXT0:
    display.print("Wakeup caused by external signal using RTC_IO");
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

  display.frontlight.setState(false);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
  esp_deep_sleep_start();
}
