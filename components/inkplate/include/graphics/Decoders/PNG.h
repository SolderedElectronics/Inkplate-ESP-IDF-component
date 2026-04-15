#ifndef DECODER_PNG_H
#define DECODER_PNG_H

#include <stdint.h>
#include <stdbool.h>

#include "pngle.h"

class Inkplate;

class PNG
{
public:
    PNG(Inkplate *inkplate);

    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);

private:
    static void drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                             const uint8_t rgba[4]);

    Inkplate       *m_inkplate;
    int             m_x;
    int             m_y;
    bool            m_invert;
    bool            m_dither;

    static PNG      *m_instance;
    static int64_t   m_lastYieldUs;
    static uint32_t  m_lastDitherY;
};

#endif
