#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "Inkplate.h"
#include "JPEG.h"

static const char* TAG = "JPEG";

JPEG *JPEG::m_instance = nullptr;

struct JpegSrc
{
  const uint8_t *data;
  uint32_t       index;
  uint32_t       size;
};

JPEG::JPEG(Inkplate *inkplate)
    : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false), m_dither(false),
      m_lastYieldUs(0), m_lineBuf(nullptr), m_lineBufH(0), m_lineBufY(0)
{
}

/**
 * @brief  Draw a JPEG image from a memory buffer.
 *
 * @param  buf     Pointer to the JPEG file data.
 * @param  len     Length of the buffer in bytes.
 * @param  x       X position of the top-left corner on the display.
 * @param  y       Y position of the top-left corner on the display.
 * @param  invert  True to invert colours.
 * @param  dither  True to apply Floyd-Steinberg dithering.
 *
 * @return true on success, false if the decoder reports an error.
 */
bool JPEG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  m_instance    = this;
  m_x           = x;
  m_y           = y;
  m_invert      = invert;
  m_dither      = dither;
  m_lastYieldUs = esp_timer_get_time();
  m_lineBuf     = nullptr;
  m_lineBufH    = 0;
  m_lineBufY    = 0;

  uint8_t *workspace = (uint8_t *)malloc(TJPGD_WORKSPACE_SIZE);
  if (!workspace)
    return false;

  JpegSrc src  = { buf, 0, (uint32_t)len };
  JDEC    jdec = {};

  JRESULT res = jd_prepare(&jdec, inputCallback, workspace, TJPGD_WORKSPACE_SIZE, &src);
  if (res != JDR_OK)
  {
    if (res == JDR_FMT3 || res == JDR_FMT2)
      ESP_LOGE(TAG, "Unsupported JPEG format. Only baseline JPEG is supported.");
    free(workspace);
    return false;
  }

  res = jd_decomp(&jdec, outputCallback, 0);

  free(m_lineBuf);
  m_lineBuf = nullptr;
  free(workspace);
  return res == JDR_OK;
}

/**
 * @brief  tjpgd input callback — feeds compressed data to the decoder.
 */
UINT JPEG::inputCallback(JDEC *jdec, BYTE *buf, UINT len)
{
  JpegSrc *src = (JpegSrc *)jdec->device;

  if (src->index + len > src->size)
    len = src->size - src->index;

  if (buf)
    memcpy(buf, src->data + src->index, len);

  src->index += len;
  return len;
}

/**
 * @brief  tjpgd output callback — called once per decoded MCU block.
 *
 * @note   The ROM decoder outputs RGB888 (3 bytes/pixel, R-G-B order).
 *
 *         Without dithering: pixels are drawn directly.
 *
 *         With dithering: pixels are stored as 8-bit luminance in a line
 *         buffer (one MCU block height tall, full image width wide).  Once
 *         the last MCU block of a row arrives, the complete rows are passed
 *         through Floyd-Steinberg sequentially so that errors carry correctly
 *         across MCU block column boundaries.
 */
UINT JPEG::outputCallback(JDEC *jdec, void *bitmap, JRECT *rect)
{
  if (!m_instance)
    return 0;

  BYTE    *px    = (BYTE *)bitmap;
  uint16_t w     = rect->right  - rect->left + 1;
  uint16_t h     = rect->bottom - rect->top  + 1;
  int      baseX = m_instance->m_x + rect->left;
  int      baseY = m_instance->m_y + rect->top;
  bool     dither = m_instance->m_dither;

  // Yield to IDLE task periodically so the task watchdog doesn't trigger
  int64_t now = esp_timer_get_time();
  if (now - m_instance->m_lastYieldUs >= 1000000LL)
  {
    vTaskDelay(1);
    m_instance->m_lastYieldUs = esp_timer_get_time();
  }

  if (dither)
  {
    // Allocate the line buffer on the first callback once we know block height and image width
    if (!m_instance->m_lineBuf)
    {
      m_instance->m_lineBufH = h;
      m_instance->m_lineBuf  = (uint8_t *)malloc(h * jdec->width);
      if (!m_instance->m_lineBuf)
        return 0;
    }

    // Store 8-bit luminance for each pixel into the line buffer
    for (uint16_t j = 0; j < h; ++j)
      for (uint16_t i = 0; i < w; ++i)
      {
        UINT idx = (j * w + i) * 3;
        m_instance->m_lineBuf[j * jdec->width + rect->left + i] =
            RGB8BIT(px[idx], px[idx + 1], px[idx + 2]);
      }

    // Once the last MCU block in this row arrives, we have complete rows —
    // apply Floyd-Steinberg row by row so errors carry across block boundaries
    if ((uint16_t)(rect->right + 1) == jdec->width)
    {
      int rowBaseY = m_instance->m_y + m_instance->m_lineBufY;

      for (int j = 0; j < m_instance->m_lineBufH; ++j)
      {
        for (int i = 0; i < (int)jdec->width; ++i)
        {
          uint8_t lum8 = m_instance->m_lineBuf[j * jdec->width + i];
          uint8_t val  = m_instance->m_inkplate->image.getDitheredPixel(
                           lum8, i, 0, jdec->width, false);

          if (m_instance->m_invert)
            val ^= 7;
          if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
            val = (~val >> 2) & 1;

          m_instance->m_inkplate->drawPixel(m_instance->m_x + i, rowBaseY + j, val);
        }
        m_instance->m_inkplate->image.ditherSwap(jdec->width);
      }

      m_instance->m_lineBufY += m_instance->m_lineBufH;
    }
  }
  else
  {
    for (uint16_t j = 0; j < h; ++j)
      for (uint16_t i = 0; i < w; ++i)
      {
        UINT    idx = (j * w + i) * 3;
        uint8_t r   = px[idx];
        uint8_t g   = px[idx + 1];
        uint8_t b   = px[idx + 2];
        uint8_t val = RGB3BIT(r, g, b);

        if (m_instance->m_invert)
          val ^= 7;
        if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
          val = (~val >> 2) & 1;

        m_instance->m_inkplate->drawPixel(baseX + i, baseY + j, val);
      }
  }

  return 1;
}
