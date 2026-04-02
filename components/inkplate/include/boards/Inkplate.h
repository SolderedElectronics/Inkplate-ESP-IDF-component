#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "graphics/Graphics.h"
#include "Image.h"
#include "Network.h"

#if CONFIG_INKPLATE_BOARD_INKPLATE6
  #include "inkplate6/Inkplate6.h"
  #define INKPLATE_BOARD_CLASS Inkplate6
#else
  #error "No Inkplate board selected. Choose a board in menuconfig → Inkplate Board."
#endif

class Inkplate : public Graphics, public INKPLATE_BOARD_CLASS
{
  public:
    Inkplate();

    void    drawPixel(int16_t x, int16_t y, uint16_t color);
    uint8_t getRotation() override;

    Image image;
    WiFi  wifi;

  private:
    void writePixel(int16_t x, int16_t y, uint16_t color);
};
#endif
