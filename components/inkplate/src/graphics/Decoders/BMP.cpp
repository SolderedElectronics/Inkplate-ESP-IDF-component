#include <string.h>
#include <stdlib.h>

#include "Inkplate.h"
#include "BMP.h"

BMP::BMP(Inkplate *inkplate) : m_inkplate(inkplate)
{
    memset(m_pixelBuffer, 0, sizeof(m_pixelBuffer));
    memset(m_paletteRGB,  0, sizeof(m_paletteRGB));
}

bool BMP::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
    BitmapHeader header;
    readHeader(buf, &header);

    if (!isValid(&header))
        return false;

    uint8_t *bufferPtr = buf + header.startRAW;
    for (int i = 0; i < header.height; ++i)
    {
        memcpy(m_pixelBuffer, bufferPtr, BMP_ROWSIZE(header.width, header.color));
        drawLine(x, y + i, &header, invert, dither);
        if (dither)
            m_inkplate->image.ditherSwap(header.width);
        bufferPtr += BMP_ROWSIZE(header.width, header.color);
    }
    return true;
}

void BMP::readHeader(uint8_t *buf, BitmapHeader *header)
{
    header->startRAW = *(uint32_t *)(buf + 10);
    header->width    = *(int32_t  *)(buf + 18);
    header->height   = abs(*(int32_t *)(buf + 22));
    header->color    = *(uint16_t *)(buf + 28);

    if (header->color <= 8)
    {
        uint32_t numColors  = 1U << header->color;
        uint8_t *colorTable = buf + 54;
        for (uint32_t i = 0; i < numColors; i++)
        {
            uint8_t b = colorTable[i * 4 + 0];
            uint8_t g = colorTable[i * 4 + 1];
            uint8_t r = colorTable[i * 4 + 2];
            m_paletteRGB[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }
    }
}

bool BMP::isValid(BitmapHeader *header)
{
    return (header->color == 1  || header->color == 4  ||
            header->color == 8  || header->color == 16 ||
            header->color == 24 || header->color == 32);
}

void BMP::drawLine(int16_t x, int16_t y, BitmapHeader *header, bool invert, bool dither)
{
    int16_t w = header->width;
    int16_t h = header->height;
    int8_t  c = header->color;

    for (int j = 0; j < w; ++j)
    {
        uint8_t r = 0, g = 0, b = 0;

        switch (c)
        {
        case 1: {
            uint8_t bit = !!(m_pixelBuffer[j >> 3] & (1 << (7 - (j & 7))));
            r = (m_paletteRGB[bit] >> 16) & 0xFF;
            g = (m_paletteRGB[bit] >>  8) & 0xFF;
            b = (m_paletteRGB[bit]      ) & 0xFF;
            break;
        }
        case 4: {
            uint8_t px = (m_pixelBuffer[j >> 1] & (j & 1 ? 0x0F : 0xF0)) >> (j & 1 ? 0 : 4);
            r = (m_paletteRGB[px] >> 16) & 0xFF;
            g = (m_paletteRGB[px] >>  8) & 0xFF;
            b = (m_paletteRGB[px]      ) & 0xFF;
            break;
        }
        case 8: {
            uint8_t px = m_pixelBuffer[j];
            r = (m_paletteRGB[px] >> 16) & 0xFF;
            g = (m_paletteRGB[px] >>  8) & 0xFF;
            b = (m_paletteRGB[px]      ) & 0xFF;
            break;
        }
        case 16: {
            uint16_t px = ((uint16_t)m_pixelBuffer[(j << 1) | 1] << 8) | m_pixelBuffer[j << 1];
            r = (px & 0x7C00) >> 7;
            g = (px & 0x03E0) >> 2;
            b = (px & 0x001F) << 3;
            break;
        }
        case 24: {
            b = m_pixelBuffer[j * 3];
            g = m_pixelBuffer[j * 3 + 1];
            r = m_pixelBuffer[j * 3 + 2];
            break;
        }
        case 32: {
            b = m_pixelBuffer[j * 4];
            g = m_pixelBuffer[j * 4 + 1];
            r = m_pixelBuffer[j * 4 + 2];
            break;
        }
        }

        uint8_t val;
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) || defined(CONFIG_INKPLATE_BOARD_INKPLATE2) || defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
        if (dither)
            val = m_inkplate->image.getDitheredPixel(r, g, b, j, w);
        else
            val = m_inkplate->image.findClosestPalette(r, g, b);
        if (invert && val < 2) val ^= 1;
#else
        // Grayscale boards: requires Image class in Inkplate.h
        if (dither)
            val = m_inkplate->image.getDitheredPixel(RGB8BIT(r, g, b), j, 0, w, false);
        else
            val = RGB3BIT(r, g, b);
        if (invert) val ^= 7;
        if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
            val = (~val >> 2) & 1;
#endif

        m_inkplate->drawPixel(x + j, h - y - 1, val);
    }
}
