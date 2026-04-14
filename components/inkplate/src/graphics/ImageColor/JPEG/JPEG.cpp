#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "Inkplate.h"
#include "JPEG.h"

static const char *TAG = "ImageColorJPEG";

ImageColorJPEG *ImageColorJPEG::m_instance = nullptr;

struct JpegColorSrc
{
  const uint8_t *data;
  uint32_t       index;
  uint32_t       size;
};

static uint8_t findClosestColor(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t dWhite = (uint32_t)(255 - r) * (255 - r) + (uint32_t)(255 - g) * (255 - g) + (uint32_t)(255 - b) * (255 - b);
  uint32_t dBlack = (uint32_t)r * r + (uint32_t)g * g + (uint32_t)b * b;
  uint32_t dRed   = (uint32_t)(255 - r) * (255 - r) + (uint32_t)g * g + (uint32_t)b * b;

  if (dWhite <= dBlack && dWhite <= dRed) return 0;
  if (dBlack <= dRed)                     return 1;
  return 2;
}

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

ImageColorJPEG::ImageColorJPEG(Inkplate *inkplate)
    : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false), m_dither(false),
      m_lastYieldUs(0), m_rgbLineBuf(nullptr), m_lineBufH(0), m_lineBufY(0)
{
}

/**
 * @brief  Draw a 3-color JPEG image from a memory buffer.
 *
 * @param  uint8_t *buf    pointer to JPEG file data
 * @param  int32_t len     buffer length in bytes
 * @param  int x, y        top-left corner on the display
 * @param  bool invert     swap black and white
 * @param  bool dither     apply Floyd-Steinberg dithering
 *
 * @return true on success, false if the decoder reports an error
 */
bool ImageColorJPEG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  m_instance    = this;
  m_x           = x;
  m_y           = y;
  m_invert      = invert;
  m_dither      = dither;
  m_lastYieldUs = esp_timer_get_time();
  m_rgbLineBuf  = nullptr;
  m_lineBufH    = 0;
  m_lineBufY    = 0;

  uint8_t *workspace = (uint8_t *)malloc(TJPGD_COLOR_WORKSPACE_SIZE);
  if (!workspace)
  {
    ESP_LOGE(TAG, "Out of memory for JPEG workspace");
    return false;
  }

  JpegColorSrc src  = { buf, 0, (uint32_t)len };
  JDEC         jdec = {};

  JRESULT res = jd_prepare(&jdec, inputCallback, workspace, TJPGD_COLOR_WORKSPACE_SIZE, &src);
  if (res != JDR_OK)
  {
    if (res == JDR_FMT3 || res == JDR_FMT2)
      ESP_LOGE(TAG, "Unsupported JPEG format. Only baseline JPEG is supported.");
    free(workspace);
    return false;
  }

  res = jd_decomp(&jdec, outputCallback, 0);

  free(m_rgbLineBuf);
  m_rgbLineBuf = nullptr;
  free(workspace);
  return res == JDR_OK;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

UINT ImageColorJPEG::inputCallback(JDEC *jdec, BYTE *buf, UINT len)
{
  JpegColorSrc *src = (JpegColorSrc *)jdec->device;

  if (src->index + len > src->size)
    len = src->size - src->index;

  if (buf)
    memcpy(buf, src->data + src->index, len);

  src->index += len;
  return len;
}

UINT ImageColorJPEG::outputCallback(JDEC *jdec, void *bitmap, JRECT *rect)
{
  if (!m_instance)
    return 0;

  BYTE    *px    = (BYTE *)bitmap;
  uint16_t w     = rect->right  - rect->left + 1;
  uint16_t h     = rect->bottom - rect->top  + 1;
  int      baseX = m_instance->m_x + rect->left;
  int      baseY = m_instance->m_y + rect->top;

  // yield to IDLE task periodically so the task watchdog doesn't trigger
  int64_t now = esp_timer_get_time();
  if (now - m_instance->m_lastYieldUs >= 1000000LL)
  {
    vTaskDelay(1);
    m_instance->m_lastYieldUs = esp_timer_get_time();
  }

  if (m_instance->m_dither)
  {
    // allocate line buffer on the first callback (once MCU block height is known)
    if (!m_instance->m_rgbLineBuf)
    {
      m_instance->m_lineBufH   = h;
      m_instance->m_rgbLineBuf = (uint8_t *)malloc(h * jdec->width * 3);
      if (!m_instance->m_rgbLineBuf)
        return 0;
    }

    // copy this block's RGB into the line buffer
    for (uint16_t j = 0; j < h; ++j)
      for (uint16_t i = 0; i < w; ++i)
      {
        UINT srcIdx = (j * w + i) * 3;
        UINT dstIdx = (j * jdec->width + rect->left + i) * 3;
        m_instance->m_rgbLineBuf[dstIdx + 0] = px[srcIdx + 0];
        m_instance->m_rgbLineBuf[dstIdx + 1] = px[srcIdx + 1];
        m_instance->m_rgbLineBuf[dstIdx + 2] = px[srcIdx + 2];
      }

    // when the last MCU block in this row has arrived, dither and draw the full rows
    if ((uint16_t)(rect->right + 1) == jdec->width)
    {
      int rowBaseY = m_instance->m_y + m_instance->m_lineBufY;
      for (int j = 0; j < m_instance->m_lineBufH; ++j)
      {
        for (int i = 0; i < (int)jdec->width; ++i)
        {
          UINT    idx = (j * jdec->width + i) * 3;
          uint8_t r   = m_instance->m_rgbLineBuf[idx + 0];
          uint8_t g   = m_instance->m_rgbLineBuf[idx + 1];
          uint8_t b   = m_instance->m_rgbLineBuf[idx + 2];
          uint8_t val = m_instance->m_inkplate->image.getDitheredPixel(r, g, b, i, jdec->width);

          if (m_instance->m_invert && val < 2) val ^= 1;

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
        uint8_t val = findClosestColor(r, g, b);

        if (m_instance->m_invert && val < 2) val ^= 1;

        m_instance->m_inkplate->drawPixel(baseX + i, baseY + j, val);
      }
  }

  return 1;
}
