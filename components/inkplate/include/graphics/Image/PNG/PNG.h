#ifndef _PNG_H_
#define _PNG_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pngle.h"

// convert RGB to 3-bit grayscale (0-7) using luminance weights
#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif

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
  static PNG     *m_instance;
  static int64_t  m_lastYieldUs;
  static uint32_t m_lastDitherY;  // tracks row changes for ditherSwap
};

#endif
