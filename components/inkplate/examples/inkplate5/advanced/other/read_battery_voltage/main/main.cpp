/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Battery voltage reading example for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to read the connected Li-ion/Li-Po battery
 *              voltage using Inkplate's built-in battery measurement circuitry.
 *              The measured voltage is shown on the e-paper display alongside
 *              a battery icon, and updated every 10 seconds.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable, 3.6-4.2 V Li-ion/Li-Po battery
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Connect a supported Li-ion/Li-Po battery to the Inkplate battery connector.
 * 2) Build and flash to Inkplate 5.
 * 3) The battery voltage is read and displayed every 10 seconds.
 *
 * Expected output:
 * - Battery icon with measured voltage displayed on screen.
 * - Updates every 10 seconds.
 *
 * Notes:
 * - readBattery() returns a double value in volts.
 * - Accuracy depends on battery condition and load.
 * - Reading requires enabling the battery measurement path in hardware.
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
#include "battSymbol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.clearDisplay();
  display.display();
  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);

  while (true) {
    double voltage = display.readBattery();
    display.clearDisplay();
    display.image.draw(battSymbol, 100, 100, 106, 45, BLACK);
    display.setCursor(230, 110);
    display.print(voltage, 2);
    display.print('V');
    display.display();
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
