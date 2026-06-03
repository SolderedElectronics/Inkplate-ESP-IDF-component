/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen event monitoring via serial for Inkplate 6 Flick.
 *
 * @details     Demonstrates how to read touch events from the Inkplate 6 Flick
 *              touchscreen and print the detected touch coordinates to the
 *              serial output. Supports multi-touch (up to two simultaneous
 *              fingers). When a touch occurs, the number of detected fingers
 *              and their coordinates are printed. When all fingers are
 *              released, a "Release" message is printed.
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
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Open a serial terminal at 115200 baud.
 * 3) Touch the display with one or two fingers.
 * 4) Coordinates (X, Y) will appear in the terminal.
 *
 * Expected behavior:
 * - When a finger touches the screen, coordinates (X,Y) are printed.
 * - If two fingers touch simultaneously, both coordinates are printed.
 * - When all fingers are lifted, "Release" is printed.
 *
 * Notes:
 * - The touchscreen supports up to two simultaneous touch points.
 * - Coordinates are automatically adjusted if display rotation changes.
 * - This example sets display rotation to orientation 2.
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

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.setRotation(2);
  display.fillTriangle(10, 10, 20, 40, 40, 20, BLACK);
  display.setTextSize(3);
  display.setCursor(60, 60);
  display.print("(0,0) position");
  display.display();

  while (true) {
    if (display.touchscreen.available()) {
      uint8_t n;
      uint16_t x[2], y[2];

      n = display.touchscreen.getData(x, y);
      if (n != 0) {
        printf("%d finger%s ", n, n > 1 ? "s" : "");
        for (int i = 0; i < n; i++)
          printf("X=%d Y=%d ", x[i], y[i]);
        printf("\n");
      } else {
        x[0] = 0; x[1] = 0;
        y[0] = 0; y[1] = 0;
        printf("Release\n");
      }
    } else {
      vTaskDelay(1);
    }
  }
}
