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

struct bitmapHeader
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
  void readHeader(uint8_t *buf, bitmapHeader *header);
  bool isValid(bitmapHeader *header);
  void drawLine(int16_t x, int16_t y, bitmapHeader *header, bool invert, bool dither);

  Inkplate *m_inkplate;

  uint8_t m_pixelBuffer[BMP_MAX_ROW_SIZE];
  uint8_t m_palette[128];
};

#endif
