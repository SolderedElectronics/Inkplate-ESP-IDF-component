#ifndef _BOARD_BASE_H_
#define _BOARD_BASE_H_

#include "I2S.h"
#include "esp_err.h"
#include <stdint.h>

typedef enum
{
    BLACK_AND_WHITE = 0,
    GRAYSCALE,
} displayMode_t;

class BoardBase : public I2S
{
public:
    virtual void setDisplayMode(displayMode_t mode) = 0;
    virtual void writePixelInternal(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void clearDisplay() = 0;
    virtual void fillDisplay() = 0;
    virtual esp_err_t display(bool leaveOn = false) = 0;
    virtual esp_err_t einkOn() = 0;
    virtual esp_err_t einkOff() = 0;
    virtual void setFullUpdateThreshold(uint16_t numberOfPartialUpdates) = 0;

    virtual ~BoardBase() = default;
};

#endif
