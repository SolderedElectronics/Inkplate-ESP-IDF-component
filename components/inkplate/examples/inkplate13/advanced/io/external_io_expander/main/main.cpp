/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       IO expander GPIO blink example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates controlling an external GPIO through the on-board
 *              PCAL IO expander. Pin P1-7 (IO_NUM_B7) is configured as an
 *              output and toggled every second. Connect an LED with a 330 Ohm
 *              current-limiting resistor to that pin.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable, 330 Ohm resistor, LED
 * - Extra:      LED connected between P1-7 (IO Expander 2 header) and GND
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Connect a 330 Ohm resistor to P1-7 on the IO Expander 2 header.
 * 2) Connect the other end of the resistor to the LED anode (+).
 * 3) Connect the LED cathode (-) to GND.
 * 4) Build and flash to Inkplate 13SPECTRA.
 * 5) The LED blinks every second.
 *
 * Expected output:
 * - LED on P1-7 toggles with 1-second intervals indefinitely.
 *
 * Notes:
 * - GPA0=0 ... GPA7=7, GPB0=8 ... GPB7=15 (IO_NUM_B7 = 15).
 * - expander1 is defined in the board driver (BoardCommon.cpp).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
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
    expander1.setLevel(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    expander1.setLevel(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
