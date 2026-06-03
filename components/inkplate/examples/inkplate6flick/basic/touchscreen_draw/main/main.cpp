/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen drawing example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to use the Inkplate 6 Flick touchscreen to
 *              draw directly on the e-paper display. Touch coordinates are
 *              read and graphics are rendered at the touched location.
 *
 *              Two drawing modes are available via a compile-time define:
 *              - DRAW_LINE:   draws a continuous line following the finger.
 *              - DRAW_CIRCLE: draws filled circles at touch points.
 *
 *              The display is refreshed using partial updates for faster
 *              drawing responsiveness.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - Select drawing mode by enabling one of:
 *     #define DRAW_LINE
 *     #define DRAW_CIRCLE
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Touch the screen to draw.
 * 3) In line mode, a continuous line follows your finger.
 * 4) In circle mode, filled circles are drawn where the screen is touched.
 *
 * Notes:
 * - Only the first detected touch point is used.
 * - Partial updates refresh only the modified area for faster interaction.
 * - Touchscreen coordinates are automatically adjusted based on display
 *   rotation.
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

#define DRAW_LINE
// #define DRAW_CIRCLE

extern "C" void app_main(void) {
  Inkplate display;

#ifdef DRAW_LINE
  uint16_t xOld = 0, yOld = 0;
#endif

  display.setDisplayMode(BLACK_AND_WHITE);
  display.display();
  
  while (true) {
    if (display.touchscreen.available()) {
      uint8_t n;
      uint16_t x[2], y[2];
      n = display.touchscreen.getData(x, y);
      if (n != 0) {
#ifdef DRAW_LINE
        display.drawLine(xOld, yOld, x[0], y[0], BLACK);
        xOld = x[0];
        yOld = y[0];
#endif

#ifdef DRAW_CIRCLE
        display.fillCircle(x[0], y[0], 20, BLACK);
#endif
        display.partialUpdate();
      }
    } else {
      vTaskDelay(1);
    }
  }
}
