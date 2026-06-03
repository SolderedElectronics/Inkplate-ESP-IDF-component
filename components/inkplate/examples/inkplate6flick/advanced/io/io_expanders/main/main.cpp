/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Internal and external IO expander control example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to control GPIO pins on both the internal and
 *              external IO expanders available on Inkplate 6 Flick. The example
 *              alternates blinking an LED connected to the external IO expander
 *              (IO Expander 2) and an LED connected to the internal IO expander
 *              (IO Expander 1), showing correct usage and addressing for each.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable, 2x LED, 2x 330 Ohm resistors
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Connect an LED + 330 Ohm resistor to P1-7 (GPB7) on IO Expander 2 (external).
 * 2) Connect another LED + 330 Ohm resistor to P1-7 (GPB7) on IO Expander 1 (internal).
 * 3) Build and flash to Inkplate 6 Flick.
 * 4) Observe alternating blinking between external and internal LEDs.
 *
 * Expected output:
 * - External IO expander LED blinks for 5 seconds.
 * - Internal IO expander LED blinks for 5 seconds.
 * - Sequence repeats continuously.
 *
 * Notes:
 * - External IO expander pins are all free to use by default.
 * - Internal IO expander has restrictions:
 *   - DO NOT use GPA0-GPA7 or GPB0.
 *   - Use only pins 9-15 (P1-1 to P1-7).
 * - Using restricted pins may permanently damage the display.
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
#include "PCAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pin P1-7 (GPB7) on both expanders
// GPA0 = IO_NUM_A0 ... GPA7 = IO_NUM_A7
// GPB0 = IO_NUM_B0 ... GPB7 = IO_NUM_B7
#define LED_PIN IO_NUM_B7

// expander2 = external IO expander (addr 0x21) — all pins free to use
extern PCAL expander2;

// WARNING: DO NOT use GPA0-GPA7 or GPB0 on expander1 — may permanently damage the display!
// Use only GPB1-GPB7 (IO_NUM_B1 to IO_NUM_B7).
extern PCAL expander1;

extern "C" void app_main(void) {
  Inkplate display;

  expander2.setDirection(LED_PIN, IO_MODE_OUTPUT);
  expander1.setDirection(LED_PIN, IO_MODE_OUTPUT);

  while (true) {
    // External IO Expander (IO Expander 2)
    for (int i = 0; i < 5; i++) {
      expander2.setLevel(LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      expander2.setLevel(LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Internal IO Expander (IO Expander 1)
    for (int i = 0; i < 5; i++) {
      expander1.setLevel(LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      expander1.setLevel(LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
