#ifndef DECODER_BMP_H
#define DECODER_BMP_H

#include <stdint.h>
#include <stdbool.h>

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#define BMP_MAX_WIDTH    1600
#else
#define BMP_MAX_WIDTH    800
#endif
#define BMP_MAX_ROW_SIZE (BMP_MAX_WIDTH * 4) // 32bpp max
#define BMP_ROWSIZE(w, c) ((((w) * (c) + 31) / 32) * 4)

#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif
#ifndef RGB8BIT
#define RGB8BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 8))
#endif

struct BitmapHeader
{
    uint32_t startRAW;
    int32_t  width;
    int32_t  height;
    uint16_t color;
};

class Inkplate;

class BMP
{
public:
    BMP(Inkplate *inkplate);

    bool draw(uint8_t *buf, int x, int y, bool invert = false, bool dither = false);

private:
    void readHeader(uint8_t *buf, BitmapHeader *header);
    bool isValid(BitmapHeader *header);
    void drawLine(int16_t x, int16_t y, BitmapHeader *header, bool invert, bool dither);

    Inkplate *m_inkplate;
    uint8_t   m_pixelBuffer[BMP_MAX_ROW_SIZE];
    uint32_t  m_paletteRGB[256]; // packed 0x00RRGGBB per indexed-BMP palette entry
};

#endif
