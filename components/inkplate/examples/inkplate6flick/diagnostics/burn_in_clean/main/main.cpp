/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Run a burn-in cleaning cycle to reduce ghosting/burn-in on the
 *              Inkplate 6 Flick e-paper panel.
 *
 * @details     This example calls Inkplate's cleanBurnIn() routine to perform a
 *              repeated full-refresh cleaning sequence intended to reduce heavy
 *              ghosting (burn-in) on the e-paper panel. The cleaning process
 *              runs for a configurable number of cycles, with a fixed delay
 *              between cycles to respect e-paper refresh limitations.
 *
 *              The display is used in 1-bit (BW) mode and the cleaning routine
 *              performs multiple full updates, which may take several minutes
 *              depending on CLEAR_CYCLES and CYCLES_DELAY. When the sequence
 *              finishes, a confirmation message is rendered on the screen.
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
 * 1) Set CLEAR_CYCLES to the number of cleaning refresh cycles you want.
 * 2) Set CYCLES_DELAY (ms) between cycles (keep it >= 5000 ms).
 * 3) Build and flash to Inkplate 6 Flick and keep the device powered for the
 *    entire process.
 * 4) Wait until the screen shows "Clearing done."
 *
 * Expected output:
 * - E-paper: The panel will repeatedly refresh during the cleaning routine.
 *   After completion, the message "Clearing done." is displayed.
 *
 * Notes:
 * - Display mode is 1-bit (BW).
 * - This routine performs many full refreshes; do not interrupt power during
 *   the cleaning sequence.
 * - Use CYCLES_DELAY >= 5 seconds to avoid overstressing the panel and to allow
 *   refresh waveforms to complete properly.
 * - Burn-in/ghosting reduction effectiveness depends on the panel condition and
 *   the content that caused the artifact; multiple runs may be required for
 *   severe cases.
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

#define CLEAR_CYCLES 20

// NOTE: cycles delay should not be smaller than 5 seconds
#define CYCLES_DELAY 5000

extern "C" void app_main(void) {
  Inkplate display;
  display.clearDisplay();

  display.cleanBurnIn(CLEAR_CYCLES, CYCLES_DELAY);

  display.setTextSize(4);
  display.setCursor(100, 100);
  display.print("Clearing done.");
  display.display();
}
