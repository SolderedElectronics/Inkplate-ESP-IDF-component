/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Internal IO expander control example for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to control GPIO pins on the internal IO
 *              expander available on Inkplate 5. The example blinks an LED
 *              connected to pin P1-7 (GPB7) on the internal IO expander,
 *              showing correct usage and pin addressing.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable, LED, 330 Ω resistor
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Connect an LED + 330 Ω resistor to P1-7 (GPB7) on the IO expander.
 * 2) Build and flash to Inkplate 5.
 * 3) Observe the LED blinking every second.
 *
 * Expected output:
 * - LED connected to IO_NUM_B7 blinks once per second continuously.
 *
 * Notes:
 * - Internal IO expander has restrictions — DO NOT use GPA0-GPA7 or GPB0.
 * - Use only pins 9-15 (P1-1 to P1-7, i.e. IO_NUM_B1 to IO_NUM_B7).
 * - Using restricted pins may permanently damage the display.
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
#include "PCAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN IO_NUM_B7

extern PCAL expander1;

extern "C" void app_main(void) {
  Inkplate display;

  expander1.setDirection(LED_PIN, IO_MODE_OUTPUT);

  while (true) {
    expander1.setLevel(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    expander1.setLevel(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
