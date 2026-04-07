#include <string.h>
#include <stdlib.h>

#include "Inkplate.h"
#include "BMP.h"

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

BMP::BMP(Inkplate *inkplate) : m_inkplate(inkplate)
{
  memset(m_pixelBuffer, 0, sizeof(m_pixelBuffer));
  memset(m_palette,     0, sizeof(m_palette));
}

/**
 * @brief  Draw a BMP image from a memory buffer.
 *
 * @param  uint8_t *buf
 *         pointer to the BMP file data
 * @param  int x
 *         X position of the top-left corner on the display
 * @param  int y
 *         Y position of the top-left corner on the display
 * @param  bool invert
 *         true to invert colours
 * @param  bool dither
 *         true to apply dithering
 *
 * @return bool
 *         true on success, false if the colour depth is not supported
 */
bool BMP::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
  bitmapHeader header;
  readHeader(buf, &header);

  if (!isValid(&header))
    return false;

  uint8_t *bufferPtr = buf + header.startRAW;
  for (int i = 0; i < header.height; ++i)
  {
    memcpy(m_pixelBuffer, bufferPtr, ROWSIZE(header.width, header.color));
    drawLine(x, y + i, &header, invert, dither);
    if (dither)
      m_inkplate->image.ditherSwap(header.width);
    bufferPtr += ROWSIZE(header.width, header.color);
  }
  return true;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Parse BMP header fields and build the palette for indexed modes.
 *
 * @note   For 1, 4 and 8 bpp BMPs the colour table starts at byte 54.
 *         Each entry is 4 bytes (B, G, R, reserved).
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

      // pack two 3-bit grayscale values per byte: high nibble = even index
      if (i & 1)
        m_palette[i >> 1] |= gray;
      else
        m_palette[i >> 1]  = gray << 4;

      // 8-bit luminance for dithering
      m_inkplate->image.m_ditherPalette[i] = RGB8BIT(r, g, b);
    }
  }
}

/**
 * @brief  Check if BMP header is valid.
 *
 * @return bool
 *         true if the colour depth is one we can decode
 */
bool BMP::isValid(bitmapHeader *header)
{
  return (header->color == 1  || header->color == 4  ||
          header->color == 8  || header->color == 16 ||
          header->color == 24 || header->color == 32);
}

/**
 * @brief  Write one line of horizontal pixels.
 *
 * @param  int16_t x
 *         top left x image position
 * @param  int16_t y
 *         top left y image position
 * @param  bitmapHeader *bmpHeader
 *         bitmap header with image data
 * @param  bool dither
 *         1 if using dither, 0 if not
 * @param  bool invert
 *         1 if using invert, 0 if not
 */
void BMP::drawLine(int16_t x, int16_t y, bitmapHeader *header, bool invert, bool dither)
{
  int16_t w = header->width;
  int16_t h = header->height;
  int8_t  c = header->color;
  bool    bw = (m_inkplate->getDisplayMode() == BLACK_AND_WHITE);

  for (int j = 0; j < w; ++j)
  {
    uint8_t val = 0;

    switch (c)
    {
    case 1: {
      uint8_t bit = !!(m_pixelBuffer[j >> 3] & (1 << (7 - (j & 7))));
      if (dither)
        val = m_inkplate->image.getDitheredPixel(bit, j, 0, w, true);
      else
        val = (invert ^ (m_palette[0] > m_palette[1])) ^ bit;
      break;
    }
    case 4: {
      uint8_t px = (m_pixelBuffer[j >> 1] & (j & 1 ? 0x0F : 0xF0)) >> (j & 1 ? 0 : 4);
      if (dither)
        val = m_inkplate->image.getDitheredPixel(px, j, 0, w, true);
      else
        val = (m_palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
      break;
    }
    case 8: {
      uint8_t px = m_pixelBuffer[j];
      if (dither)
        val = m_inkplate->image.getDitheredPixel(px, j, 0, w, true);
      else
        val = (m_palette[px >> 1] & (px & 1 ? 0x0F : 0xF0)) >> (px & 1 ? 0 : 4);
      break;
    }
    case 16: {
      uint16_t px = ((uint16_t)m_pixelBuffer[(j << 1) | 1] << 8) | m_pixelBuffer[(j << 1)];
      uint8_t  r  = (px & 0x7C00) >> 7;
      uint8_t  g  = (px & 0x03E0) >> 2;
      uint8_t  b  = (px & 0x001F) << 3;
      if (dither)
        val = m_inkplate->image.getDitheredPixel(RGB8BIT(r, g, b), j, 0, w, false);
      else
        val = RGB3BIT(r, g, b);
      break;
    }
    case 24: {
      uint8_t b = m_pixelBuffer[j * 3];
      uint8_t g = m_pixelBuffer[j * 3 + 1];
      uint8_t r = m_pixelBuffer[j * 3 + 2];
      if (dither)
        val = m_inkplate->image.getDitheredPixel(RGB8BIT(r, g, b), j, 0, w, false);
      else
        val = RGB3BIT(r, g, b);
      break;
    }
    case 32: {
      uint8_t b = m_pixelBuffer[j * 4];
      uint8_t g = m_pixelBuffer[j * 4 + 1];
      uint8_t r = m_pixelBuffer[j * 4 + 2];
      if (dither)
        val = m_inkplate->image.getDitheredPixel(RGB8BIT(r, g, b), j, 0, w, false);
      else
        val = RGB3BIT(r, g, b);
      break;
    }
    }

    if (invert && c != 1) val ^= 7;
    if (bw) val = (~val >> 2) & 1;

    m_inkplate->drawPixel(x + j, (h - y - 1), val);
  }
}
