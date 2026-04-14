#ifndef _IMAGE_COLOR_BMP_H_
#define _IMAGE_COLOR_BMP_H_

#include <stdint.h>
#include <stdbool.h>

#define BMP_COLOR_MAX_WIDTH    800
#define BMP_COLOR_MAX_ROW_SIZE (BMP_COLOR_MAX_WIDTH * 4) // 32 bpp max

// BMP row size in bytes, padded to 4-byte boundary (BMP spec)
#define COLOR_ROWSIZE(w, c) ((((w) * (c) + 31) / 32) * 4)

struct bitmapHeaderColor
{
  uint32_t startRAW;
  int32_t  width;
  int32_t  height;
  uint16_t color;
};

class Inkplate;

class ImageColorBMP
{
public:
  ImageColorBMP(Inkplate *inkplate);

  bool draw(uint8_t *buf, int x, int y, bool invert = false, bool dither = false);

private:
  void    readHeader(uint8_t *buf, bitmapHeaderColor *header);
  bool    isValid(bitmapHeaderColor *header);
  void    drawLine(int16_t x, int16_t y, bitmapHeaderColor *header, bool invert, bool dither);
  uint8_t findClosestColor(uint8_t r, uint8_t g, uint8_t b);

  Inkplate *m_inkplate;

  uint8_t m_pixelBuffer[BMP_COLOR_MAX_ROW_SIZE];
  uint8_t m_palette[256]; // one Inkplate2 color index per palette entry
};

#endif
