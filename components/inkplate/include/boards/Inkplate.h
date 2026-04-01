#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "Inkplate6.h"
#include "graphics/Graphics.h"

class Inkplate : public Graphics, public Inkplate6
{
  public:
    Inkplate();

    void    drawPixel(int16_t x, int16_t y, uint16_t color);
    uint8_t getRotation() override;

  private:
    void writePixel(int16_t x, int16_t y, uint16_t color);
};
#endif
