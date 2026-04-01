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

uint8_t Inkplate::getRotation()
{
    return rotation;
}
