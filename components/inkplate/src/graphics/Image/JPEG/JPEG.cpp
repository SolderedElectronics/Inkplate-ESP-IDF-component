#include "Inkplate.h"
#include "JPEG.h"

JPEG *JPEG::m_instance = nullptr;

JPEG::JPEG(Inkplate *inkplate) : m_inkplate(inkplate)
{
}

/**
 * @brief  Draw a JPEG image from a memory buffer.
 *
 * @param  buf    Pointer to the JPEG file data.
 * @param  len    Length of the buffer in bytes.
 * @param  x      X position of the top-left corner on the display.
 * @param  y      Y position of the top-left corner on the display.
 * @param  invert True to invert colours.
 *
 * @return true on success, false if TJpgDec reports an error.
 */
bool JPEG::draw(uint8_t *buf, int32_t len, int x, int y, bool invert)
{
    m_instance = this;

    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(drawChunk);

    return TJpgDec.drawJpg(x, y, buf, len, false, invert) == JDR_OK;
}

/**
 * @brief  TJpgDec render callback — called once per decoded MCU block.
 *
 * @note   Each pixel in @p bitmap is RGB565. We convert it to 3-bit grayscale
 *         (or 1-bit in B&W mode) and forward it to drawPixel.
 */
bool JPEG::drawChunk(int16_t x, int16_t y, uint16_t w, uint16_t h,
                     uint16_t *bitmap, bool /*dither*/, bool invert)
{
    if (!m_instance)
        return false;

    for (int j = 0; j < h; ++j)
    {
        for (int i = 0; i < w; ++i)
        {
            uint16_t px  = bitmap[j * w + i];
            uint8_t  r   = JPEG_R(px);
            uint8_t  g   = JPEG_G(px);
            uint8_t  b   = JPEG_B(px);
            uint8_t  val = RGB3BIT(r, g, b);

            if (invert)
                val = 7 - val;
            if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
                val = (~val >> 2) & 1;

            m_instance->m_inkplate->drawPixel(x + i, y + j, val);
        }
    }

    return true;
}
