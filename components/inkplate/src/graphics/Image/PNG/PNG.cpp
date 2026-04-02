#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "Inkplate.h"
#include "PNG.h"

PNG    *PNG::m_instance   = nullptr;
int64_t PNG::m_lastYieldUs = 0;

PNG::PNG(Inkplate *inkplate) : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false)
{
}

/**
 * @brief  Draw a PNG image from a memory buffer.
 *
 * @param  buf     Pointer to the PNG file data.
 * @param  len     Length of the buffer in bytes.
 * @param  x       X position of the top-left corner on the display.
 * @param  y       Y position of the top-left corner on the display.
 * @param  invert  True to invert colours.
 *
 * @return true on success, false if pngle reports a decode error.
 */
bool PNG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert)
{
    m_instance    = this;
    m_x           = x;
    m_y           = y;
    m_invert      = invert;
    m_lastYieldUs = esp_timer_get_time();

    pngle_t *pngle = pngle_new();
    if (!pngle)
        return false;

    pngle_set_draw_callback(pngle, drawCallback);

    int result = pngle_feed(pngle, buf, len);

    pngle_destroy(pngle);

    return result >= 0;
}

/**
 * @brief  pngle draw callback — called once per pixel (or small block).
 *
 * @note   Fully transparent pixels (rgba[3] == 0) are skipped.
 *         1-bit depth images are expanded to full black/white before conversion.
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

    uint8_t val = RGB3BIT(r, g, b);

    if (m_instance->m_invert)
        val = 7 - val;
    if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
        val = (~val >> 2) & 1;

    for (uint32_t j = 0; j < h; ++j)
        for (uint32_t i = 0; i < w; ++i)
            m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i, m_instance->m_y + y + j, val);
}
