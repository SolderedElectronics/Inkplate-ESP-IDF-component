/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads and programs the Inkplate 6 Flick EPD VCOM voltage.
 *
 * @details     This example shows how to read the currently stored VCOM value
 *              from the display power IC/EEPROM and optionally program a new
 *              VCOM value. After programming, a simple grayscale test pattern
 *              is drawn and the stored VCOM value is shown on the e-paper
 *              display.
 *
 *              VCOM is stored in EEPROM and can only be programmed a limited
 *              number of times. Do NOT run this sketch repeatedly or "tune"
 *              VCOM by trial-and-error. Program it once (only if needed) and
 *              leave it unchanged to avoid prematurely wearing out EEPROM.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Set the desired VCOM value in the VCOM_VALUE define.
 * 2) Build and flash to Inkplate 6 Flick.
 * 3) The programmed VCOM value will be shown on the display alongside
 *    a grayscale test pattern to verify display quality.
 *
 * Expected output:
 * - E-paper: The stored VCOM voltage value and a grayscale gradient test
 *   pattern.
 *
 * Notes:
 * - VCOM should typically be set once during manufacturing or initial setup.
 * - Incorrect VCOM values can cause poor contrast or display artifacts.
 * - WARNING: VCOM is written to EEPROM with limited write endurance. Avoid
 *   repeated programming to prevent permanent wear/damage.
 * - Refer to your panel's datasheet for the correct VCOM value.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include <stdio.h>

#include "Inkplate.h"

#define VCOM_VALUE (-2.35)

static void display_test_image(Inkplate &display) {
  display.clearDisplay();

  double vcom = display.getStoredVCOM();

  display.setTextColor(0);
  display.setTextSize(2);
  display.setCursor(5, 5);
  display.print("Stored VCOM: ");
  display.print(vcom);
  display.print(" V");

  int w = display.width() / 8;
  int h = display.height();

  for (int i = 0; i < 8; i++) {
    int x = w * i;
    display.fillRect(x, 40, w, h, i);
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  printf("Setting VCOM to %.2f\n", VCOM_VALUE);

  if (display.setVCOM(VCOM_VALUE) == ESP_OK)
    printf("VCOM programmed OK\n");
  else
    printf("VCOM programming failed\n");

  display_test_image(display);
}
