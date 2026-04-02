#include <string.h>
#include <stdlib.h>

#include "Inkplate.h"
#include "BMP.h"

BMP::BMP(Inkplate *inkplate) : m_inkplate(inkplate)
{
    memset(m_pixelBuffer, 0, sizeof(m_pixelBuffer));
    memset(m_palette,     0, sizeof(m_palette));
}

/**
 * @brief  Draw a BMP image from a memory buffer.
 *
 * @param  uint8_t *buf   Pointer to the BMP file data.
 * @param  int x          X position of the top-left corner on the display.
 * @param  int y          Y position of the top-left corner on the display.
 * @param  bool invert    True to invert colours.
 *
 * @return true on success, false if the colour depth is not supported.
 */
bool BMP::draw(uint8_t *buf, int x, int y, bool invert)
{
    bitmapHeader header;
    readHeader(buf, &header);

    if (!isValid(&header))
        return false;

    uint8_t *bufferPtr = buf + header.startRAW;
    for (int i = 0; i < header.height; ++i)
    {
        memcpy(m_pixelBuffer, bufferPtr, ROWSIZE(header.width, header.color));
        drawLine(x, y + i, &header, invert);
        bufferPtr += ROWSIZE(header.width, header.color);
    }
    return true;
}

/**
 * @brief  Parse BMP header fields and build the palette for indexed modes.
 *
 * @note   For 1, 4 and 8 bpp BMPs the colour table starts at byte 54.
 *         Each entry is 4 bytes (B, G, R, reserved). We convert each entry
 *         to a 3-bit grayscale value and pack two entries per byte.
 */
void BMP::readHeader(uint8_t *buf, bitmapHeader *header)
{
    header->startRAW = *(uint32_t *)(buf + 10);
    header->width    = *(int32_t  *)(buf + 18);
    header->height   = abs(*(int32_t *)(buf + 22));
    header->color    = *(uint16_t *)(buf + 28);

    if (header->color <= 8)
    {
        uint32_t numColors  = 1U << header->color;
        uint8_t *colorTable = buf + 54;
        memset(m_palette, 0, sizeof(m_palette));

        for (uint32_t i = 0; i < numColors; i++)
        {
            uint8_t b    = colorTable[i * 4 + 0];
            uint8_t g    = colorTable[i * 4 + 1];
            uint8_t r    = colorTable[i * 4 + 2];
            uint8_t gray = RGB3BIT(r, g, b);

            // Pack two 3-bit grayscale values per byte: high nibble = even index
            if (i & 1)
                m_palette[i >> 1] |= gray;
            else
                m_palette[i >> 1]  = gray << 4;
        }
    }
}

/**
 * @brief  Return true if the colour depth is one we can decode.
 */
bool BMP::isValid(bitmapHeader *header)
{
    return (header->color == 1  || header->color == 4  ||
            header->color == 8  || header->color == 16 ||
            header->color == 24 || header->color == 32);
}

/**
 * @brief  Draw one decoded row to the display.
 *
 * @note   BMP rows are stored bottom-up, so row i maps to display row (h - i - 1).
 */
void BMP::drawLine(int16_t x, int16_t y, bitmapHeader *header, bool invert)
{
    int16_t w = header->width;
    int16_t h = header->height;
    int8_t  c = header->color;

    for (int j = 0; j < w; ++j)
    {
        switch (c)
        {
        case 1: {
            uint8_t val = (invert ^ (m_palette[0] > m_palette[1])) ^
                          !!(m_pixelBuffer[j >> 3] & (1 << (7 - (j & 7))));
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        case 4: {
            uint8_t px  = (m_pixelBuffer[j >> 1] & (j & 1 ? 0x0F : 0xF0)) >> (j & 1 ? 0 : 4);
            uint8_t val = (m_palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
            if (invert) val ^= 7;
            if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        case 8: {
            uint8_t px  = m_pixelBuffer[j];
            uint8_t val = (m_palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
            if (invert) val ^= 7;
            if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        case 16: {
            uint16_t px  = ((uint16_t)m_pixelBuffer[(j << 1) | 1] << 8) | m_pixelBuffer[(j << 1)];
            uint8_t  r   = (px & 0x7C00) >> 7;
            uint8_t  g   = (px & 0x03E0) >> 2;
            uint8_t  b   = (px & 0x001F) << 3;
            uint8_t  val = RGB3BIT(r, g, b);
            if (invert) val ^= 7;
            if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        case 24: {
            uint8_t b   = m_pixelBuffer[j * 3];
            uint8_t g   = m_pixelBuffer[j * 3 + 1];
            uint8_t r   = m_pixelBuffer[j * 3 + 2];
            uint8_t val = RGB3BIT(r, g, b);
            if (invert) val ^= 7;
            if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        case 32: {
            uint8_t b   = m_pixelBuffer[j * 4];
            uint8_t g   = m_pixelBuffer[j * 4 + 1];
            uint8_t r   = m_pixelBuffer[j * 4 + 2];
            uint8_t val = RGB3BIT(r, g, b);
            if (invert) val = 7 - val;
            if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;
            m_inkplate->drawPixel(x + j, (h - y - 1), val);
            break;
        }
        }
    }
}
