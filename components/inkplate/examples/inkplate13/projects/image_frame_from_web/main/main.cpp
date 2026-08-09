/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       TODO: port from Arduino example "Inkplate13SPECTRA_Image_Frame_From_Web" (Image Frame From Web) to
 *              ESP-IDF for Soldered Inkplate 13.
 *
 * @details     Scaffold only - not yet ported. See the original Arduino
 *              sketch under
 *              "SolderedElectronics Inkplate-Arduino-library/examples/Inkplate13SPECTRA/Projects/Inkplate13SPECTRA_Image_Frame_From_Web/"
 *              for the source logic to port.
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"

extern "C" void app_main(void) {
  // TODO: port Image Frame From Web from Inkplate13SPECTRA_Image_Frame_From_Web.
}
