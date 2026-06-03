/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen touch-in-area demo for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to use the Inkplate 6 Flick touchscreen by
 *              detecting touches inside a defined rectangular area. A filled
 *              rectangle is drawn on the display; when the user touches inside
 *              the rectangle, it moves diagonally. Partial updates are used for
 *              fast redraws, with a full refresh when the rectangle resets.
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
 * 2) A black rectangle appears on the screen.
 * 3) Touch inside the rectangle to move it diagonally across the display.
 * 4) When it reaches the lower area, the rectangle resets to the start.
 *
 * Expected output:
 * - Rectangle moves when touched inside its bounds.
 * - Partial updates used for fast movement; full refresh on reset.
 *
 * Notes:
 * - Touch detection uses touchscreen.touchInArea(x, y, w, h).
 * - Touchscreen is initialized and powered on with touchscreen.init(true).
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

  int x_position = 50;
  int y_position = 50;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.setCursor(100, 300);
  display.setTextSize(3);
  display.print("Touch button example. Touch the black button.");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(3000));
  display.clearDisplay();

  display.fillRect(x_position, y_position, 100, 50, BLACK);
  display.display();

  while (true) {
    if (display.touchscreen.touchInArea(x_position, y_position, 100, 50)) {
      x_position += 100;
      y_position += 100;

      if (y_position < 660) {
        display.clearDisplay();
        display.fillRect(x_position, y_position, 100, 50, BLACK);
        display.partialUpdate();
        vTaskDelay(pdMS_TO_TICKS(100));
      } else {
        x_position = 50;
        y_position = 50;

        display.clearDisplay();
        display.fillRect(x_position, y_position, 100, 50, BLACK);
        display.display();
      }
    } else {
      vTaskDelay(1);
    }
  }
}
