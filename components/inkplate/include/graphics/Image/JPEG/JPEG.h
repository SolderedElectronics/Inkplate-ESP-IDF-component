#ifndef _JPEG_H_
#define _JPEG_H_

#include <stdint.h>
#include <stdbool.h>

#include "esp32/rom/tjpgd.h"

// workspace size for the TJpgDec decompressor
#define TJPGD_WORKSPACE_SIZE 16688

// convert RGB to 3-bit grayscale (0-7) using luminance weights
#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif

class Inkplate;

class JPEG
{
public:
  JPEG(Inkplate *inkplate);

  bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);

private:
  static UINT inputCallback(JDEC *jdec, BYTE *buf, UINT len);
  static UINT outputCallback(JDEC *jdec, void *bitmap, JRECT *rect);

  Inkplate    *m_inkplate;
  int          m_x;
  int          m_y;
  bool         m_invert;
  bool         m_dither;
  int64_t      m_lastYieldUs;
  uint8_t     *m_lineBuf;    // one MCU-row-height of luminance values (dither only)
  int          m_lineBufH;   // MCU block height, set from first callback
  int          m_lineBufY;   // current absolute Y offset in the image
  static JPEG *m_instance;
};

#endif
