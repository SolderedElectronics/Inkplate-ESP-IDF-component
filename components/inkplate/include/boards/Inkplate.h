/**
 **************************************************
 *
 * @file        Inkplate.h
 * @brief       Basic funtions for controling inkplate
 *
 *              https://github.com/e-radionicacom/Inkplate-Arduino-library
 *              For support, please reach over forums: forum.e-radionica.com/en
 *              For more info about the product, please check: www.inkplate.io
 *
 *              This code is released under the GNU Lesser General Public
 *License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html Please review the
 *LICENSE file included with this example. If you have any questions about
 *licensing, please contact techsupport@e-radionica.com Distributed as-is; no
 *warranty is given.
 *
 * @authors     @ Soldered
 ***************************************************/

#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "Inkplate6.h"
#include "graphics/Graphics.h"

class Inkplate : public Graphics, public Inkplate6
{
  public:
    Inkplate();

    void begin();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void setRotation(uint8_t r);


  protected:
  private:
    uint8_t _rotation = 0;
    uint8_t _beginDone = 0;
    uint8_t _mode;
    void writePixel(int16_t x, int16_t y, uint16_t color);
};
#endif
