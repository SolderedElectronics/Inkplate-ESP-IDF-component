#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "Inkplate.h"
#include "PNG.h"

PNG     *PNG::m_instance   = nullptr;
int64_t  PNG::m_lastYieldUs = 0;
uint32_t PNG::m_lastDitherY = UINT32_MAX;

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

PNG::PNG(Inkplate *inkplate) : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false), m_dither(false)
{
}

/**
 * @brief  Draw a PNG image from a memory buffer.
 *
 * @param  uint8_t *buf
 *         pointer to the PNG file data
 * @param  int32_t len
 *         length of the buffer in bytes
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
 *         true on success, false if pngle reports a decode error.
 */
bool PNG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert, bool dither)
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

/**
 * @brief  pngle draw callback — called once per pixel (or small block).
 *
 * @param  pngle_t *pngle
 *         pointer to image
 * @param  uint32_t x
 *         x plane position
 * @param  uint32_t y
 *         y plane position
 * @param  uint32_t w
 *         image width
 * @param  uint32_t h
 *         image height
 * @param  uint8_t rgba[4]
 *         color
 */
void PNG::drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
             const uint8_t rgba[4])
{
  if (!m_instance)
    return;

  // Yield to IDLE task periodically so the task watchdog doesn't trigger
  int64_t now = esp_timer_get_time();
  if (now - m_instance->m_lastYieldUs >= 1000000LL)
  {
    vTaskDelay(1);
    m_instance->m_lastYieldUs = esp_timer_get_time();
  }

  // Skip fully transparent pixels
  if (!rgba[3])
    return;

  uint8_t r = rgba[0];
  uint8_t g = rgba[1];
  uint8_t b = rgba[2];

  // 1-bit depth: expand to full black or white
  pngle_ihdr_t *ihdr = pngle_get_ihdr(pngle);
  if (ihdr->depth == 1)
    r = g = b = (b ? 0xFF : 0);

  uint8_t val;
  if (m_instance->m_dither)
  {
    // Flush the previous row's errors into the next row before moving on
    if (y != m_lastDitherY && m_lastDitherY != UINT32_MAX)
      m_instance->m_inkplate->image.ditherSwap(ihdr->width);
    m_lastDitherY = y;

    val = m_instance->m_inkplate->image.getDitheredPixel(RGB8BIT(r, g, b), x, 0, ihdr->width, false);
  }
  else
  {
    val = RGB3BIT(r, g, b);
  }

  if (m_instance->m_invert)
    val ^= 7;
  if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
    val = (~val >> 2) & 1;

  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; ++i)
      m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i, m_instance->m_y + y + j, val);
}
