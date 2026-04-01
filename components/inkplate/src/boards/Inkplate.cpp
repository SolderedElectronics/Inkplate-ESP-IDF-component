/**
 **************************************************
 *
 * @file        Inkplate.cpp
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

#include "Inkplate.h"

Inkplate::Inkplate() : Adafruit_GFX(E_INK_WIDTH, E_INK_HEIGHT), Graphics(E_INK_WIDTH, E_INK_HEIGHT)
{
}

void Inkplate::begin()
{
    if (_beginDone == 1)
        return;

    // Init low level driver for EPD.
    Inkplate6::begin();

    // Clean frame buffers.
    clearDisplay();

    // Block multiple inits.
    _beginDone = 1;
}

void Inkplate::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    writePixel(x, y, color);
}

void Inkplate::writePixel(int16_t x, int16_t y, uint16_t color)
{
    writePixelInternal(x, y, color);
}

void Inkplate::setRotation(uint8_t r)
{
    rotation = (r & 3);
    switch (rotation)
    {
    case 0:
    case 2:
        _width = E_INK_WIDTH;
        _height = E_INK_HEIGHT;
        break;
    case 1:
    case 3:
        _width = E_INK_HEIGHT;
        _height = E_INK_WIDTH;
        break;
    }
}
