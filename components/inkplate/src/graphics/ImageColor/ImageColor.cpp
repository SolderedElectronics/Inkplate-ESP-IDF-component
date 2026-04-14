#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#include "Inkplate.h"
#include "ImageColor.h"

static const char *TAG = "ImageColor";

// Inkplate2 palette RGB values (index = color code)
static const uint8_t PALETTE_R[3] = {255,   0, 255}; // white, black, red
static const uint8_t PALETTE_G[3] = {255,   0,   0};
static const uint8_t PALETTE_B[3] = {255,   0,   0};

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

ImageColor::ImageColor(Inkplate *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate)
{
  memset(m_ditherPalette, 0, sizeof(m_ditherPalette));
  m_ditherR[0] = m_ditherR[1] = nullptr;
  m_ditherG[0] = m_ditherG[1] = nullptr;
  m_ditherB[0] = m_ditherB[1] = nullptr;
}

uint8_t ImageColor::findClosestPalette(int16_t r, int16_t g, int16_t b)
{
    int32_t minDist = INT32_MAX;
    uint8_t best = 0;

    for (uint8_t i = 0; i < PALETTE_SIZE; i++)
    {
        int32_t dr = r - RED8(pallete[i]);
        int32_t dg = g - GREEN8(pallete[i]);;
        int32_t db = b - BLUE8(pallete[i]);;

        int32_t dist = dr*dr + dg*dg + db*db;

        if (dist < minDist)
        {
            minDist = dist;
            best = i;
        }
    }

    return best;
}

void ImageColor::setDitherKernel(DitherKernel kernel)
{
    if ((uint8_t)kernel < DITHER_KERNEL_COUNT)
        m_currentKernel = &DITHER_KERNELS[(uint8_t)kernel];
}

/**
 * @brief  Compute the dithered 3-colour value for one pixel using
 *         Floyd-Steinberg error diffusion on separate R, G, B channels.
 *
 * @param  r, g, b   Input colour.
 * @param  i         Column index (0-based) within the current row.
 * @param  w         Row width in pixels.
 *
 * @return Inkplate2 colour index: 0=white, 1=black, 2=red.
 */
uint8_t ImageColor::getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i, int w)
{
  // accumulate error into input channels
  int16_t er = (int16_t)r + m_ditherR[0][i];
  int16_t eg = (int16_t)g + m_ditherG[0][i];
  int16_t eb = (int16_t)b + m_ditherB[0][i];

  // clear consumed error slot
  m_ditherR[0][i] = 0;
  m_ditherG[0][i] = 0;
  m_ditherB[0][i] = 0;

  // clamp
  if (er < 0) er = 0; else if (er > 255) er = 255;
  if (eg < 0) eg = 0; else if (eg > 255) eg = 255;
  if (eb < 0) eb = 0; else if (eb > 255) eb = 255;

  uint8_t closest = findClosestPalette((uint8_t)er, (uint8_t)eg, (uint8_t)eb);

  // quantisation errors per channel
  int32_t rErr = r - (int32_t)((pallete[closest] >> 16) & 0xFF);
  int32_t gErr = g - (int32_t)((pallete[closest] >> 8) & 0xFF);
  int32_t bErr = b - (int32_t)((pallete[closest] >> 0) & 0xFF);

  const DitherKernelDef *k = m_currentKernel;

  // kernel origin offset
  int originX = k->x;

  // iterate kernel rows
  for (int ky = 0; ky < k->height; ky++)
  {
      int rowIndex = (ky == 0) ? 0 : 1; // current row or next row

      for (int kx = 0; kx < k->width; kx++)
      {
          int weight = k->data[ky * k->width + kx];
          if (weight == 0) continue;

          int offsetX = kx - originX;
          int targetX = i + offsetX;

          if (targetX < 0 || targetX >= w) continue;

          // Skip current pixel itself
          if (ky == 0 && offsetX <= 0) continue;

          m_ditherR[rowIndex][targetX] += (int16_t)((rErr * weight) / k->coef);
          m_ditherG[rowIndex][targetX] += (int16_t)((gErr * weight) / k->coef);
          m_ditherB[rowIndex][targetX] += (int16_t)((bErr * weight) / k->coef);
      }
  }

  return closest;
}

/**
 * @brief  Advance the dither error buffer to the next row.
 */
void ImageColor::ditherSwap(int w)
{
  // swap pointers so "next row" becomes "current row"
  int16_t *tmpR = m_ditherR[0]; m_ditherR[0] = m_ditherR[1]; m_ditherR[1] = tmpR;
  int16_t *tmpG = m_ditherG[0]; m_ditherG[0] = m_ditherG[1]; m_ditherG[1] = tmpG;
  int16_t *tmpB = m_ditherB[0]; m_ditherB[0] = m_ditherB[1]; m_ditherB[1] = tmpB;

  // zero out the new "next row"
  memset(m_ditherR[1], 0, (w + 2) * sizeof(int16_t));
  memset(m_ditherG[1], 0, (w + 2) * sizeof(int16_t));
  memset(m_ditherB[1], 0, (w + 2) * sizeof(int16_t));
}

/**
 * @brief  Draw a 3-color BMP image whose size is embedded in the file header.
 */
bool ImageColor::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
  if (buf[0] != 0x42 || buf[1] != 0x4D)
  {
    ESP_LOGE(TAG, "Unrecognised image format (supply length for JPEG/PNG)");
    return false;
  }
  if (dither) beginDither();
  bool result = m_bmp.draw(buf, x, y, invert, dither);
  if (dither) endDither();
  return result;
}

/**
 * @brief  Draw a 3-color image from a buffer with an explicit length.
 *         Supports BMP, JPEG and PNG.
 */
bool ImageColor::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  if (dither) beginDither();

  bool result = false;
  if (buf[0] == 0xFF && buf[1] == 0xD8)
    result = m_jpeg.draw(buf, len, x, y, invert, dither);
  else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47)
    result = m_png.draw(buf, len, x, y, invert, dither);
  else if (buf[0] == 0x42 && buf[1] == 0x4D)
    result = m_bmp.draw(buf, x, y, invert, dither);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither) endDither();
  return result;
}

/**
 * @brief  Draw a 3-color image from a URL (HTTP or HTTPS).
 *
 * @note   SD card paths are not supported on this board.
 */
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

  bool result = draw(buf, len, x, y, invert, dither);
  free(buf);
  return result;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

void ImageColor::beginDither()
{
  for (int i = 0; i < 2; ++i)
  {
    m_ditherR[i] = (int16_t *)calloc(BMP_COLOR_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherG[i] = (int16_t *)calloc(BMP_COLOR_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherB[i] = (int16_t *)calloc(BMP_COLOR_MAX_WIDTH + 2, sizeof(int16_t));
  }
}

void ImageColor::endDither()
{
  for (int i = 0; i < 2; ++i)
  {
    free(m_ditherR[i]); m_ditherR[i] = nullptr;
    free(m_ditherG[i]); m_ditherG[i] = nullptr;
    free(m_ditherB[i]); m_ditherB[i] = nullptr;
  }
}
