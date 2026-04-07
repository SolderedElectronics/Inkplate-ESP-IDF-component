#ifndef _INKPLATE_10_H_
#define _INKPLATE_10_H_

#include "BoardBase.h"
#include "pins.h"

#define E_INK_WIDTH  1200
#define E_INK_HEIGHT 825

class Inkplate10 : public BoardBase
{
public:
    Inkplate10();

    void          setDisplayMode(displayMode_t mode);
    displayMode_t getDisplayMode() { return m_displayMode; }
    void          writePixelInternal(int16_t x, int16_t y, uint16_t color);
    void          clearDisplay();
    void          fillDisplay();
    esp_err_t     display(bool leaveOn = false);
    esp_err_t     einkOn();
    esp_err_t     einkOff();

private:
    displayMode_t m_displayMode = GRAYSCALE;
};

#endif
