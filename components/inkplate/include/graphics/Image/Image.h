#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "BMP/BMP.h"
#include "JPEG/JPEG.h"
#include "PNG/PNG.h"

// convert RGB to 8-bit grayscale (0-255) using luminance weights (for dithering)
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
  friend class BMP;

  void      beginDither();
  void      endDither();

  Inkplate *m_inkplate;
  BMP       m_bmp;
  JPEG      m_jpeg;
  PNG       m_png;

  bool      m_dither;
  uint8_t  *m_ditherBuffer[2];
  uint8_t   m_ditherPalette[256];
};

#endif
