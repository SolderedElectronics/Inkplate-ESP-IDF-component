#ifndef INKPLATE_H
#define INKPLATE_H

#include "graphics/Graphics.h"
#include "Network.h"
#include "sdkconfig.h"

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2) || defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) || defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#include "ImageColor.h"
#else
#include "Image.h"
#endif

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6) || defined(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK) 
  #include "Inkplate6.h"
  #define INKPLATE_BOARD_CLASS Inkplate6
#elif CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  #include "Inkplate6color.h"
  #define INKPLATE_BOARD_CLASS Inkplate6Color
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "Inkplate10.h"
  #define INKPLATE_BOARD_CLASS Inkplate10
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
  #include "Inkplate13.h"
  #define INKPLATE_BOARD_CLASS Inkplate13
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
  #include "Inkplate5.h"
  #define INKPLATE_BOARD_CLASS Inkplate5
#elif CONFIG_INKPLATE_BOARD_INKPLATE4
  #include "Inkplate4.h"
  #define INKPLATE_BOARD_CLASS Inkplate4
#elif CONFIG_INKPLATE_BOARD_INKPLATE2
  #include "Inkplate2.h"
  #define INKPLATE_BOARD_CLASS Inkplate2
#else
  #error "No Inkplate board selected. Choose a board in menuconfig -> Inkplate Board."
#endif

class Inkplate : public Graphics, public INKPLATE_BOARD_CLASS
{
  public:
  Inkplate();

  void    drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getRotation() override;

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2) || defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) || defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
  ImageColor image;
#else
  Image image;
#endif
  WiFi  wifi;

  private:
  void writePixel(int16_t x, int16_t y, uint16_t color);
};
#endif
