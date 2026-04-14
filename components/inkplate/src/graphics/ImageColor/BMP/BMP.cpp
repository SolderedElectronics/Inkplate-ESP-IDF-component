#include <string.h>
#include <stdlib.h>

#include "Inkplate.h"
#include "BMP.h"

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

ImageColorBMP::ImageColorBMP(Inkplate *inkplate) : m_inkplate(inkplate)
{
  memset(m_pixelBuffer, 0, sizeof(m_pixelBuffer));
  memset(m_palette,     0, sizeof(m_palette));
}

/**
 * @brief  Draw a 3-color BMP image from a memory buffer.
 *
 * @param  uint8_t *buf    pointer to BMP file data
 * @param  int x, y        top-left corner on the display
 * @param  bool invert     swap black and white
 * @param  bool dither     apply Floyd-Steinberg dithering
 *
 * @return true on success, false if colour depth is unsupported
 */
bool ImageColorBMP::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
  bitmapHeaderColor header;
  readHeader(buf, &header);

  if (!isValid(&header))
    return false;

  uint8_t *bufferPtr = buf + header.startRAW;
  for (int i = 0; i < header.height; ++i)
  {
    memcpy(m_pixelBuffer, bufferPtr, COLOR_ROWSIZE(header.width, header.color));
    drawLine(x, y + i, &header, invert, dither);
    if (dither)
      m_inkplate->image.ditherSwap(header.width);
    bufferPtr += COLOR_ROWSIZE(header.width, header.color);
  }
  return true;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Parse BMP header and build the palette for indexed modes.
 *
 * @note   For indexed BMPs (1/4/8 bpp), both m_palette (closest colour index)
 *         and imageColor.m_ditherPalette (original packed RGB) are populated
 *         so that the dither path has access to the true source colour.
 */
void ImageColorBMP::readHeader(uint8_t *buf, bitmapHeaderColor *header)
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
      uint8_t b = colorTable[i * 4 + 0];
      uint8_t g = colorTable[i * 4 + 1];
      uint8_t r = colorTable[i * 4 + 2];
      m_palette[i] = findClosestColor(r, g, b);
      m_inkplate->image.m_ditherPalette[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
  }
}

bool ImageColorBMP::isValid(bitmapHeaderColor *header)
{
  return (header->color == 1  || header->color == 4  ||
          header->color == 8  || header->color == 16 ||
          header->color == 24 || header->color == 32);
}

void ImageColorBMP::drawLine(int16_t x, int16_t y, bitmapHeaderColor *header, bool invert, bool dither)
{
  int16_t w = header->width;
  int16_t h = header->height;
  int8_t  c = header->color;

  for (int j = 0; j < w; ++j)
  {
    uint8_t val = 0;

    switch (c)
    {
    case 1: {
      uint8_t bit = !!(m_pixelBuffer[j >> 3] & (1 << (7 - (j & 7))));
      if (dither) {
        uint32_t rgb = m_inkplate->image.m_ditherPalette[bit];
        val = m_inkplate->image.getDitheredPixel((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, j, w);
      } else {
        val = m_palette[bit];
      }
      break;
    }
    case 4: {
      uint8_t px = (m_pixelBuffer[j >> 1] >> (j & 1 ? 0 : 4)) & 0x0F;
      if (dither) {
        uint32_t rgb = m_inkplate->image.m_ditherPalette[px];
        val = m_inkplate->image.getDitheredPixel((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, j, w);
      } else {
        val = m_palette[px];
      }
      break;
    }
    case 8: {
      uint8_t px = m_pixelBuffer[j];
      if (dither) {
        uint32_t rgb = m_inkplate->image.m_ditherPalette[px];
        val = m_inkplate->image.getDitheredPixel((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, j, w);
      } else {
        val = m_palette[px];
      }
      break;
    }
    case 16: {
      uint16_t px = ((uint16_t)m_pixelBuffer[(j << 1) | 1] << 8) | m_pixelBuffer[j << 1];
      uint8_t  r  = (px & 0x7C00) >> 7;
      uint8_t  g  = (px & 0x03E0) >> 2;
      uint8_t  b  = (px & 0x001F) << 3;
      val = dither ? m_inkplate->image.getDitheredPixel(r, g, b, j, w) : findClosestColor(r, g, b);
      break;
    }
    case 24: {
      uint8_t b = m_pixelBuffer[j * 3];
      uint8_t g = m_pixelBuffer[j * 3 + 1];
      uint8_t r = m_pixelBuffer[j * 3 + 2];
      val = dither ? m_inkplate->image.getDitheredPixel(r, g, b, j, w) : findClosestColor(r, g, b);
      break;
    }
    case 32: {
      uint8_t b = m_pixelBuffer[j * 4];
      uint8_t g = m_pixelBuffer[j * 4 + 1];
      uint8_t r = m_pixelBuffer[j * 4 + 2];
      val = dither ? m_inkplate->image.getDitheredPixel(r, g, b, j, w) : findClosestColor(r, g, b);
      break;
    }
    }

    if (invert && val < 2) val ^= 1;

    m_inkplate->drawPixel(x + j, (h - y - 1), val);
  }
}

uint8_t ImageColorBMP::findClosestColor(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t dWhite = (uint32_t)(255 - r) * (255 - r) + (uint32_t)(255 - g) * (255 - g) + (uint32_t)(255 - b) * (255 - b);
  uint32_t dBlack = (uint32_t)r * r + (uint32_t)g * g + (uint32_t)b * b;
  uint32_t dRed   = (uint32_t)(255 - r) * (255 - r) + (uint32_t)g * g + (uint32_t)b * b;

  if (dWhite <= dBlack && dWhite <= dRed) return 0;
  if (dBlack <= dRed)                     return 1;
  return 2;
}
