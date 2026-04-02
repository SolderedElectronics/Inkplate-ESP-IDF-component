#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "BMP/BMP.h"

class Inkplate;

class Image
{
public:
    Image(Inkplate *inkplate);

    bool draw(uint8_t *buf, int x, int y, bool invert = false);

private:
    BMP m_bmp;
};

#endif
