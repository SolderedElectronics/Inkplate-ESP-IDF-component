#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdbool.h>

#include "BMP.h"
#include "JPEG.h"
#include "PNG.h"

#ifndef RGB8BIT
#define RGB8BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 8))
#endif

class Inkplate;

class Image
{
public:
  Image(Inkplate *inkplate);

  bool      draw(uint8_t *buf, int x, int y, bool invert = false, bool dither = false);
  bool      draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);
  bool      draw(const char *src, int x, int y, bool invert = false, bool dither = false);

  // called by format decoders via m_inkplate->image when dithering is enabled
  uint8_t   getDitheredPixel(uint32_t px, int i, int j, int w, bool paletted);
  void      ditherSwap(int w);

private:
  void      beginDither();
  void      endDither();

  Inkplate *m_inkplate;
  BMP       m_bmp;
  JPEG      m_jpeg;
  PNG       m_png;

  bool      m_dither;
  uint8_t  *m_ditherBuffer[2];
};

#endif
