#include "Inkplate.h"

Inkplate::Inkplate() : Adafruit_GFX(E_INK_WIDTH, E_INK_HEIGHT), Graphics(E_INK_WIDTH, E_INK_HEIGHT)
{
    clearDisplay();
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
