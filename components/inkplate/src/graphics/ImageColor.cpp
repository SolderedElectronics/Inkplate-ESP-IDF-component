/**
 * @file   ImageColor.cpp
 * @author Fran Fodor for Soldered
 * @brief  Drawing color images.
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "esp_log.h"

#include "Inkplate.h"
#include "ImageColor.h"

static const char *TAG = "ImageColor";

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

ImageColor::ImageColor(Inkplate *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate)
{
  for (int i = 0; i < DITHER_ROW_COUNT; ++i)
    m_ditherR[i] = m_ditherG[i] = m_ditherB[i] = nullptr;
  m_rowIdx = 0;

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
  palletteSize  = 7;
  pallete[0] = 0x000000; pallete[1] = 0xFFFFFF; pallete[2] = 0x00FF00;
  pallete[3] = 0x0000FF; pallete[4] = 0xFF0000; pallete[5] = 0xFFFF00; pallete[6] = 0xFF8000;
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
  palletteSize  = 6;
  pallete[0] = 0x000000; pallete[1] = 0xFFFFFF; pallete[2] = 0xFFFF00;
  pallete[3] = 0xFF0000; pallete[4] = 0x0000FF; pallete[5] = 0x00FF00;
#else
  palletteSize  = 3;
  pallete[0] = 0xFFFFFF; pallete[1] = 0x000000; pallete[2] = 0xFF0000;
#endif
}

uint8_t ImageColor::findClosestPalette(int16_t r, int16_t g, int16_t b)
{
    int32_t minDistance = INT32_MAX;
    uint8_t contenderCount = 0;
    uint8_t contenderList[7]; // sized for max palette (7 colors)

    for (uint8_t i = 0; i < palletteSize; ++i)
    {
        int16_t dr = r - (int16_t)RED8(pallete[i]);
        int16_t dg = g - (int16_t)GREEN8(pallete[i]);
        int16_t db = b - (int16_t)BLUE8(pallete[i]);

        int32_t currentDistance = dr*dr + dg*dg + db*db;

        if (currentDistance < minDistance)
        {
            minDistance = currentDistance;
            contenderList[0] = i;
            contenderCount = 1;
        }
        else if (currentDistance == minDistance)
        {
            if (contenderCount < palletteSize)
                contenderList[contenderCount++] = i;
        }
    }

    return contenderList[0];
}

void ImageColor::setDitherKernel(DitherKernel kernel)
{
    if ((uint8_t)kernel < DITHER_KERNEL_COUNT)
        m_currentKernel = &DITHER_KERNELS[(uint8_t)kernel];
}

uint8_t ImageColor::getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i, int w)
{
  const int rowIdx = m_rowIdx & DITHER_ROW_MASK;
  int16_t *rowR = m_ditherR[rowIdx];
  int16_t *rowG = m_ditherG[rowIdx];
  int16_t *rowB = m_ditherB[rowIdx];

  int16_t er = (int16_t)r + rowR[i];
  int16_t eg = (int16_t)g + rowG[i];
  int16_t eb = (int16_t)b + rowB[i];

  rowR[i] = 0;
  rowG[i] = 0;
  rowB[i] = 0;

  if (er < 0) er = 0; else if (er > 255) er = 255;
  if (eg < 0) eg = 0; else if (eg > 255) eg = 255;
  if (eb < 0) eb = 0; else if (eb > 255) eb = 255;

  int closest = findClosestPalette(er, eg, eb);

  int32_t rErr = er - (int32_t)RED8(pallete[closest]);
  int32_t gErr = eg - (int32_t)GREEN8(pallete[closest]);
  int32_t bErr = eb - (int32_t)BLUE8(pallete[closest]);

  const DitherKernelDef *k = m_currentKernel;
  const int minOffset = (i < (int)k->x) ? -i : -(int)k->x;
  const int maxOffset = ((int)k->width - k->x - 1 < w - 1 - i) ? (int)k->width - k->x - 1 : w - 1 - i;

  for (int ky = 0; ky < k->height; ++ky)
  {
      const int nextRowIdx = (rowIdx + ky) & DITHER_ROW_MASK;
      int16_t *nextRowR = m_ditherR[nextRowIdx];
      int16_t *nextRowG = m_ditherG[nextRowIdx];
      int16_t *nextRowB = m_ditherB[nextRowIdx];
      for (int l = minOffset; l <= maxOffset; ++l)
      {
          const int weight = k->data[ky * k->width + (l + k->x)];
          if (!weight)
              continue;
          const int idx = i + l;
          nextRowR[idx] += (int16_t)((weight * rErr) / k->coef);
          nextRowG[idx] += (int16_t)((weight * gErr) / k->coef);
          nextRowB[idx] += (int16_t)((weight * bErr) / k->coef);
      }
  }

  return (uint8_t)closest;
}

void ImageColor::ditherSwap()
{
  m_rowIdx = (m_rowIdx + 1) & DITHER_ROW_MASK;
}

bool ImageColor::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
  if (buf[0] != 0x42 || buf[1] != 0x4D)
  {
    ESP_LOGE(TAG, "Unrecognised image format (supply length for JPEG/PNG)");
    return false;
  }
  if (dither) beginDither();
  bool result = m_bmp.draw(buf, x, y, dither, invert);
  if (dither) endDither();
  return result;
}

bool ImageColor::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  if (dither) beginDither();

  bool result = false;
  if (buf[0] == 0xFF && buf[1] == 0xD8)
    result = m_jpeg.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47)
    result = m_png.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x42 && buf[1] == 0x4D)
    result = m_bmp.draw(buf, x, y, dither, invert);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither) endDither();
  return result;
}

bool ImageColor::draw(const char *src, int x, int y, bool invert, bool dither)
{
  int32_t  len = 0;
  uint8_t *buf = nullptr;

  if (strncmp(src, "https://", 8) == 0)
  {
    buf = m_inkplate->wifi.downloadFileHTTPS(src, &len);
    if (!buf)
    {
      ESP_LOGE(TAG, "Failed to download: %s", src);
      return false;
    }
  }
  else if (strncmp(src, "http://", 7) == 0)
  {
    buf = m_inkplate->wifi.downloadFile(src, &len);
    if (!buf)
    {
      ESP_LOGE(TAG, "Failed to download: %s", src);
      return false;
    }
  }
  else
  {
    char fullPath[256];
    if (src[0] == '/')
      snprintf(fullPath, sizeof(fullPath), "%s", src);
    else
      snprintf(fullPath, sizeof(fullPath), "%s/%s", m_inkplate->getMountPoint(), src);

    FILE *f = fopen(fullPath, "rb");
    if (!f)
    {
      ESP_LOGE(TAG, "Failed to open: %s", fullPath);
      return false;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = (uint8_t *)malloc(len);
    if (!buf)
    {
      fclose(f);
      ESP_LOGE(TAG, "Out of memory (%ld bytes)", len);
      return false;
    }

    fread(buf, 1, len, f);
    fclose(f);
  }

  bool result = draw(buf, len, x, y, dither, invert);
  free(buf);
  return result;
}

bool ImageColor::draw(const uint8_t *buf, int w, int h, int x, int y, bool invert, bool dither)
{
  if (dither) beginDither();

  const int bytesPerRow = (w + 3) / 4;
  for (int row = 0; row < h; ++row)
  {
      for (int col = 0; col < w; ++col)
      {
          int byteIdx = row * bytesPerRow + col / 4;
          int shift   = 6 - (col % 4) * 2;
          uint8_t px  = (buf[byteIdx] >> shift) & 0x03;

          // Map 2bpp -> RGB using simple greyscale ramp
          uint8_t luma = px * 85;
          if (invert) luma = 255 - luma;

          uint8_t out;
          if (dither)
              out = getDitheredPixel(luma, luma, luma, col, w);
          else
              out = findClosestPalette(luma, luma, luma);

          m_inkplate->drawPixel(x + col, y + row, out);
      }
      if (dither) ditherSwap(w);
  }

  if (dither) endDither();
  return true;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void ImageColor::beginDither()
{
  m_rowIdx = 0;
  for (int i = 0; i < DITHER_ROW_COUNT; ++i)
  {
    m_ditherR[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherG[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherB[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
  }
}

void ImageColor::endDither()
{
  for (int i = 0; i < DITHER_ROW_COUNT; ++i)
  {
    free(m_ditherR[i]); m_ditherR[i] = nullptr;
    free(m_ditherG[i]); m_ditherG[i] = nullptr;
    free(m_ditherB[i]); m_ditherB[i] = nullptr;
  }
}
