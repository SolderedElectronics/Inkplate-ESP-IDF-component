#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "Inkplate.h"
#include "PNG.h"

ImageColorPNG *ImageColorPNG::m_instance    = nullptr;
int64_t        ImageColorPNG::m_lastYieldUs  = 0;
uint32_t       ImageColorPNG::m_lastDitherY  = UINT32_MAX;

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

ImageColorPNG::ImageColorPNG(Inkplate *inkplate)
    : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false), m_dither(false)
{
}

/**
 * @brief  Draw a 3-color PNG image from a memory buffer.
 *
 * @param  uint8_t *buf    pointer to PNG file data
 * @param  int32_t len     buffer length in bytes
 * @param  int x, y        top-left corner on the display
 * @param  bool invert     swap black and white
 * @param  bool dither     apply Floyd-Steinberg dithering
 *
 * @return true on success, false if pngle reports a decode error
 */
bool ImageColorPNG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
{
  m_instance    = this;
  m_x           = x;
  m_y           = y;
  m_invert      = invert;
  m_dither      = dither;
  m_lastYieldUs = esp_timer_get_time();
  m_lastDitherY = UINT32_MAX;

  pngle_t *pngle = pngle_new();
  if (!pngle)
    return false;

  pngle_set_draw_callback(pngle, drawCallback);

  int result = pngle_feed(pngle, buf, len);

  pngle_destroy(pngle);

  return result >= 0;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

void ImageColorPNG::drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
               const uint8_t rgba[4])
{
  if (!m_instance)
    return;

  // yield to IDLE task periodically so the task watchdog doesn't trigger
  int64_t now = esp_timer_get_time();
  if (now - m_instance->m_lastYieldUs >= 1000000LL)
  {
    vTaskDelay(1);
    m_instance->m_lastYieldUs = esp_timer_get_time();
  }

  // skip fully transparent pixels
  if (!rgba[3])
    return;

  uint8_t r = rgba[0];
  uint8_t g = rgba[1];
  uint8_t b = rgba[2];

  pngle_ihdr_t *ihdr = pngle_get_ihdr(pngle);

  // 1-bit depth: expand to full black or white
  if (ihdr->depth == 1)
    r = g = b = (b ? 0xFF : 0);

  if (m_instance->m_dither)
  {
    for (uint32_t j = 0; j < h; ++j)
    {
      uint32_t rowY = y + j;

      // advance the error buffer when moving to a new row
      if (rowY != m_lastDitherY && m_lastDitherY != UINT32_MAX)
        m_instance->m_inkplate->image.ditherSwap(ihdr->width);
      m_lastDitherY = rowY;

      for (uint32_t i = 0; i < w; ++i)
      {
        uint8_t val = m_instance->m_inkplate->image.getDitheredPixel(r, g, b, x + i, ihdr->width);
        if (m_instance->m_invert && val < 2) val ^= 1;
        m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i, m_instance->m_y + rowY, val);
      }
    }
  }
  else
  {
    uint8_t val = findClosestColor(r, g, b);
    if (m_instance->m_invert && val < 2) val ^= 1;

    for (uint32_t j = 0; j < h; ++j)
      for (uint32_t i = 0; i < w; ++i)
        m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i, m_instance->m_y + y + j, val);
  }
}
