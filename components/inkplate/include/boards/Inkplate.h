#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "graphics/Graphics.h"
#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#include "Image.h"
#endif
#include "Network.h"
#include "sdkconfig.h"

#if CONFIG_INKPLATE_BOARD_INKPLATE6
  #include "inkplate6/Inkplate6.h"
  #define INKPLATE_BOARD_CLASS Inkplate6
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "inkplate10/Inkplate10.h"
  #define INKPLATE_BOARD_CLASS Inkplate10
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
  #include "inkplate5/Inkplate5.h"
  #define INKPLATE_BOARD_CLASS Inkplate5
#elif CONFIG_INKPLATE_BOARD_INKPLATE4
  #include "inkplate4/Inkplate4.h"
  #define INKPLATE_BOARD_CLASS Inkplate4
#elif CONFIG_INKPLATE_BOARD_INKPLATE2
  #include "inkplate2/Inkplate2.h"
  #define INKPLATE_BOARD_CLASS Inkplate2
#else
  #error "No Inkplate board selected. Choose a board in menuconfig -> Inkplate Board."
#endif

class Inkplate : public Graphics, public INKPLATE_BOARD_CLASS
{
  public:
  Inkplate();

  void    drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getRotation();

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
  Image image;
#endif
  WiFi  wifi;

  private:
  void writePixel(int16_t x, int16_t y, uint16_t color);
};
#endif
