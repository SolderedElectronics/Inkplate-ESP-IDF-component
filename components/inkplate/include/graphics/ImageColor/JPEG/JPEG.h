#ifndef _IMAGE_COLOR_JPEG_H_
#define _IMAGE_COLOR_JPEG_H_

#include <stdint.h>
#include <stdbool.h>

#include "esp32/rom/tjpgd.h"

#define TJPGD_COLOR_WORKSPACE_SIZE 16688

class Inkplate;

class ImageColorJPEG
{
public:
  ImageColorJPEG(Inkplate *inkplate);

  bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);

private:
  static UINT inputCallback(JDEC *jdec, BYTE *buf, UINT len);
  static UINT outputCallback(JDEC *jdec, void *bitmap, JRECT *rect);

  Inkplate           *m_inkplate;
  int                 m_x;
  int                 m_y;
  bool                m_invert;
  bool                m_dither;
  int64_t             m_lastYieldUs;
  uint8_t            *m_rgbLineBuf; // RGB line buffer for dithering (h * width * 3 bytes)
  int                 m_lineBufH;   // MCU block height
  int                 m_lineBufY;   // current absolute Y offset in the image
  static ImageColorJPEG *m_instance;
};

#endif
