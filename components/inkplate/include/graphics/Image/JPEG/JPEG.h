#ifndef _JPEG_H_
#define _JPEG_H_

#include <stdint.h>
#include <stdbool.h>

#include "TJpg_Decoder.h"

// Convert RGB to 3-bit grayscale (0-7) using luminance weights
#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif

// Extract 8-bit colour channels from a 16-bit RGB565 word
#define JPEG_R(px) (((px) >> 8) & 0xF8)
#define JPEG_G(px) (((px) >> 3) & 0xFC)
#define JPEG_B(px) (((px) << 3) & 0xF8)

class Inkplate;

class JPEG
{
public:
    JPEG(Inkplate *inkplate);

    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);

private:
    static bool drawChunk(int16_t x, int16_t y, uint16_t w, uint16_t h,
                          uint16_t *bitmap, bool dither, bool invert);

    Inkplate    *m_inkplate;
    static JPEG *m_instance;
};

#endif
