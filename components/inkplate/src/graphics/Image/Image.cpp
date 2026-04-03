#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#include "Inkplate.h"
#include "Image.h"

static const char *TAG = "Image";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

Image::Image(Inkplate *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate)
{
  m_dither           = false;
  m_ditherBuffer[0]  = m_ditherBuffer[1] = nullptr;
  memset(m_ditherPalette, 0, sizeof(m_ditherPalette));
}

/**
 * @brief  Calculates dither for given pixel in bmp images.
 *
 * @param  uint32_t px
 *         pixel value with color information
 * @param  int i
 *         ditherBuffer width plane position
 * @param  int w
 *         image width
 * @param  bool paletted
 *         1 if paletted image, 0 if not
 *
 * @return new pixel value (dithered pixel)
 */
uint8_t Image::getDitheredPixel(uint32_t px, int i, int j, int w, bool paletted)
{
  if (paletted)
    px = m_ditherPalette[px];

  if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
    px = (uint16_t)px >> 1;

  uint16_t sum     = (uint16_t)m_ditherBuffer[0][i] + (uint16_t)px;
  uint8_t oldPixel = sum > 0xFF ? 0xFF : (uint8_t)sum;

  uint8_t newPixel   = oldPixel & (m_inkplate->getDisplayMode() == BLACK_AND_WHITE ? 0x80 : 0xE0);
  uint8_t quantError = oldPixel - newPixel;

  // distribute quantisation error to right, below-left, below, below-right
  m_ditherBuffer[1][i] += (quantError * 5) >> 4;
  if (i != w - 1)
  {
    m_ditherBuffer[0][i + 1] += (quantError * 7) >> 4;
    m_ditherBuffer[1][i + 1] += (quantError * 1) >> 4;
  }
  if (i != 0)
    m_ditherBuffer[1][i - 1] += (quantError * 3) >> 4;

  return newPixel >> 5;
}

/**
 * @brief  Swaps ditherBuffer values.
 *
 * @param  int w
 *         screen width
 */
void Image::ditherSwap(int w)
{
  for (int i = 0; i < w; ++i)
  {
    m_ditherBuffer[0][i] = m_ditherBuffer[1][i];
    m_ditherBuffer[1][i] = 0;
  }
}

/**
 * @brief  Draw an image whose size is embedded in the file header (BMP).
 */
bool Image::draw(uint8_t *buf, int x, int y, bool invert, bool dither)
{
  m_dither = dither;
  if (dither) beginDither();

  bool result = false;
  if (buf[0] == 0x42 && buf[1] == 0x4D)
    result = m_bmp.draw(buf, x, y, invert, dither);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither) endDither();
  m_dither = false;
  return result;
}

/**
 * @brief  Draw an image where the buffer length must be supplied (JPEG / PNG / BMP).
 */
bool Image::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  m_dither = dither;
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
  m_dither = false;
  return result;
}

/**
 * @brief  Draw an image from a URL or an SD card path.
 *
 * @note   Prefix selects source:
 *           "https://" → HTTPS download
 *           "http://"  → HTTP download
 *           anything else → SD card (mount point prepended if path is relative)
 */
bool Image::draw(const char *src, int x, int y, bool invert, bool dither)
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

/**
 * @brief  Allocate and zero the dither row buffers before a dithered draw call.
 */
void Image::beginDither()
{
  m_ditherBuffer[0] = (uint8_t *)calloc(BMP_MAX_WIDTH + 2, 1);
  m_ditherBuffer[1] = (uint8_t *)calloc(BMP_MAX_WIDTH + 2, 1);
}

/**
 * @brief  Free the dither row buffers after a dithered draw call completes.
 */
void Image::endDither()
{
  free(m_ditherBuffer[0]);
  free(m_ditherBuffer[1]);
  m_ditherBuffer[0] = m_ditherBuffer[1] = nullptr;
}

