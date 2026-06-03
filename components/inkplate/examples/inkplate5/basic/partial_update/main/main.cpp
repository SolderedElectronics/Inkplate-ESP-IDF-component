/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Scrolling text via partial update for Soldered Inkplate 5.
 *
 * @details     Demonstrates partial display updates by scrolling a text string
 *              across the screen from right to left. Only the changed region is
 *              refreshed each frame, which is significantly faster than a full
 *              display update. A full refresh is forced after every
 *              setFullUpdateThreshold() partial updates to prevent ghosting.
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
 * 2) The text scrolls continuously from right to left.
 *
 * Expected output:
 * - "This is partial update on Inkplate 5 e-paper display!" scrolling across
 *   the screen.
 *
 * Notes:
 * - partialUpdate(false, true) keeps e-paper power on for faster successive
 *   updates.
 * - A full refresh is triggered automatically after 9 partial updates.
 * - Partial update is only supported in BLACK_AND_WHITE display mode.
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

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"

const char text[] = "This is partial update on Inkplate 5 e-paper display! :)";

int offset = 1280;

int partialUpdates = 9;

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(4);
  display.setTextWrap(false);
  /*
  Set the number of partial updates before doing a full update.
  This function forces a full update as the next update to ensure that the
  cycle of partial updates starts from a fully updated screen. The Inkplate
  class keeps an internal counter that increments every time partialUpdate()
  gets called.
  */
  display.setFullUpdateThreshold(partialUpdates);

  while (true) {
    display.clearDisplay();
    display.setCursor(offset, 340);
    display.print(text);

    /*
    Updates changed parts of the screen without refreshing the whole display.
    partialUpdate(bool _forced, bool leaveOn)
        _forced   Can force partial update in deep sleep (for advanced use)
        leaveOn   If set to 1, disables turning off e-ink power supply after
                  update to increase refresh speed
    */
    display.partialUpdate(false, true);
    offset -= 20;
    if (offset < 0)
      offset = 1280;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
