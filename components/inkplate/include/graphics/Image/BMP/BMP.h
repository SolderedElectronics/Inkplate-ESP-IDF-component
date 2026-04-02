#ifndef _BMP_H_
#define _BMP_H_

#include <stdint.h>
#include <stdbool.h>

#define BMP_MAX_WIDTH    800
#define BMP_MAX_ROW_SIZE (BMP_MAX_WIDTH * 4) // 32bpp max

// Row size in bytes, padded to a 4-byte boundary (BMP spec)
#define ROWSIZE(w, c)    ((((w) * (c) + 31) / 32) * 4)

// Convert RGB to 3-bit grayscale (0-7) using luminance weights
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))

/**
 * @brief BMP file header fields needed for decoding.
 */
struct bitmapHeader
{
    uint32_t startRAW; // Offset to pixel data
    int32_t  width;    // Image width in pixels
    int32_t  height;   // Image height in pixels
    uint16_t color;    // Bits per pixel
};

class Inkplate;

/**
 * @brief Decoder for BMP images. Supports 1, 4, 8, 16, 24 and 32 bpp.
 */
class BMP
{
public:
    BMP(Inkplate *inkplate);

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
    bool draw(uint8_t *buf, int x, int y, bool invert = false);

private:
    /**
     * @brief  Parse BMP header fields and build the palette for indexed modes.
     */
    void readHeader(uint8_t *buf, bitmapHeader *header);

    /**
     * @brief  Return true if the colour depth is supported.
     */
    bool isValid(bitmapHeader *header);

    /**
     * @brief  Draw one decoded row to the display.
     */
    void drawLine(int16_t x, int16_t y, bitmapHeader *header, bool invert);

    Inkplate *m_inkplate;

    uint8_t m_pixelBuffer[BMP_MAX_ROW_SIZE]; // Holds one decoded row
    uint8_t m_palette[128];                  // 256 x 4-bit grayscale entries, packed 2 per byte
};

#endif
