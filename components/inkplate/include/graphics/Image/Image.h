#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "BMP/BMP.h"
#include "JPEG/JPEG.h"
#include "PNG/PNG.h"

class Inkplate;

class Image
{
public:
    Image(Inkplate *inkplate);

    bool draw(uint8_t *buf, int x, int y, bool invert = false);
    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);
    bool draw(const char *src, int x, int y, bool invert = false);

private:
    Inkplate *m_inkplate;
    BMP       m_bmp;
    JPEG      m_jpeg;
    PNG       m_png;
};

#endif
