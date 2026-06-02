/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       On-board temperature sensor reading example for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to read temperature data from the on-board
 *              temperature sensor integrated inside the TPS65186 e-paper PMIC.
 *              This sensor is intended primarily for internal compensation and
 *              basic monitoring. It is not a high-accuracy sensor and should
 *              not be used for precise temperature measurements.
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
 * 2) The program reads the temperature from the onboard PMIC sensor.
 * 3) The measured value is shown on the display and updated every 10 seconds.
 *
 * Expected output:
 * - Thermometer icon with temperature in Celsius displayed on screen.
 * - Updates every 10 seconds.
 *
 * Notes:
 * - readTemperature() returns an int8_t value in degrees Celsius.
 * - The TPS65186 PMIC sensor is intended for system monitoring and waveform
 *   compensation, not precision measurement.
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
#include "tempSymbol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.clearDisplay();
  display.display();
  display.setTextSize(4);
  display.setTextColor(BLACK, WHITE);

  while (true) {
    int8_t temperature = display.readTemperature();
    display.clearDisplay();
    display.image.draw(tempSymbol, 100, 100, 38, 79, BLACK);
    display.setCursor(155, 125);
    display.print(temperature);
    display.print('C');
    display.display();
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
