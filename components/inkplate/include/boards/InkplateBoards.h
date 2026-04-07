#ifndef _INKPLATE_BOARDS_H_
#define _INKPLATE_BOARDS_H_

#include "sdkconfig.h"

#if CONFIG_INKPLATE_BOARD_INKPLATE6
  #include "inkplate6/Inkplate6.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "inkplate10/Inkplate10.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
  #include "inkplate5/Inkplate5.h"
#else
  #error "No Inkplate board selected. Choose a board in menuconfig -> Inkplate Board."
#endif

#endif
