#ifndef _JPEG_H_
#define _JPEG_H_

#include <stdint.h>
#include <stdbool.h>

#include "TJpg_Decoder.h"

// Convert RGB to 3-bit grayscale (0-7) using luminance weights
#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif

// Extract 8-bit colour channels from a 16-bit RGB565 word
#define JPEG_R(px) (((px) >> 8) & 0xF8)
#define JPEG_G(px) (((px) >> 3) & 0xFC)
#define JPEG_B(px) (((px) << 3) & 0xF8)

class Inkplate;

/**
 * @brief Decoder for JPEG images using the TJpgDec library.
 *
 * @note  Dithering is not supported — pixels are converted directly from RGB565
 *        to 3-bit grayscale (or 1-bit in B&W mode).
 */
class JPEG
{
public:
    JPEG(Inkplate *inkplate);

    /**
     * @brief  Draw a JPEG image from a memory buffer.
     *
     * @param  buf     Pointer to the JPEG file data.
     * @param  len     Length of the buffer in bytes.
     * @param  x       X position of the top-left corner on the display.
     * @param  y       Y position of the top-left corner on the display.
     * @param  invert  True to invert colours.
     *
     * @return true on success, false if the decoder reports an error.
     */
    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);

private:
    /**
     * @brief  TJpgDec render callback — called once per decoded MCU block.
     *
     * @param  x       Block X origin on the display.
     * @param  y       Block Y origin on the display.
     * @param  w       Block width in pixels.
     * @param  h       Block height in pixels.
     * @param  bitmap  RGB565 pixel data for this block.
     * @param  dither  Unused — dithering is not implemented.
     * @param  invert  True to invert colours.
     *
     * @return true to continue decoding, false to abort.
     */
    static bool drawChunk(int16_t x, int16_t y, uint16_t w, uint16_t h,
                          uint16_t *bitmap, bool dither, bool invert);

    Inkplate    *m_inkplate;

    /** Pointer to the active JPEG instance, used by the static callback. */
    static JPEG *m_instance;
};

#endif
