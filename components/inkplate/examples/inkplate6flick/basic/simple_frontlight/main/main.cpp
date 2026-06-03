/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Frontlight control demo for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to enable and control the frontlight on the
 *              Inkplate 6 Flick. Brightness can be adjusted by sending
 *              characters through the serial monitor (UART0 at 115200 baud):
 *              '+' increases brightness, '-' decreases it, and 's' triggers a
 *              light animation sweep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick with integrated frontlight, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Open a serial terminal at 115200 baud.
 * 3) Send the following characters to control brightness:
 *      '+' -> Increase frontlight intensity
 *      '-' -> Decrease frontlight intensity
 *      's' -> Run a simple frontlight animation
 *
 * Expected output:
 * - Frontlight turns on at startup.
 * - Current brightness level (0-63) printed to serial after each change.
 *
 * Notes:
 * - Frontlight brightness range is 0-63.
 * - display.frontlight.setState(true) enables the frontlight driver circuit.
 * - display.frontlight.setBrightness(value) sets brightness level.
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
#include "freertos/FreeRTOS.h"
#include <stdio.h>

extern "C" void app_main(void) {
  Inkplate display;

  int b = 31;

  display.frontlight.setState(true);
  display.frontlight.setBrightness(b);

  printf("Frontlight example. Send '+' to increase, '-' to decrease, 's' for lightshow.\n");
  printf("Current brightness: %d/63\n", b);

  while (true) {
    int c = getchar();
    if (c == EOF) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    bool change = false;

    if (c == '+' && b < 63) {
      b++;
      change = true;
    }
    if (c == '-' && b > 0) {
      b--;
      change = true;
    }
    if (c == 's') {
      for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 64; i++) {
          display.frontlight.setBrightness(i);
          vTaskDelay(pdMS_TO_TICKS(30));
        }
        for (int i = 63; i >= 0; i--) {
          display.frontlight.setBrightness(i);
          vTaskDelay(pdMS_TO_TICKS(30));
        }
      }
      change = true;
    }

    if (change) {
      display.frontlight.setBrightness(b);
      printf("Frontlight:%d/63\n", b);
    }
  }
}
