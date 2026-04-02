#ifndef _PNG_H_
#define _PNG_H_

#include <stdint.h>
#include <stdbool.h>

#include "pngle.h"

// Convert RGB to 3-bit grayscale (0-7) using luminance weights
#ifndef RGB3BIT
#define RGB3BIT(r, g, b) ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 + (uint32_t)(b) * 19) >> 13))
#endif

class Inkplate;

/**
 * @brief Decoder for PNG images using the pngle library.
 *
 * @note  Alpha-transparent pixels (rgba[3] == 0) are skipped.
 *        Dithering is not supported.
 */
class PNG
{
public:
    PNG(Inkplate *inkplate);

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
    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);

private:
    /**
     * @brief  pngle draw callback — called once per pixel (or small block).
     *
     * @param  pngle  Decoder context (used to query IHDR).
     * @param  x      Pixel X within the image.
     * @param  y      Pixel Y within the image.
     * @param  w      Block width (usually 1).
     * @param  h      Block height (usually 1).
     * @param  rgba   RGBA colour of this pixel.
     */
    static void drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                             const uint8_t rgba[4]);

    Inkplate   *m_inkplate;
    int         m_x;       ///< Display X offset for the current draw call
    int         m_y;       ///< Display Y offset for the current draw call
    bool        m_invert;  ///< Invert flag for the current draw call

    /** Pointer to the active PNG instance, used by the static callback. */
    static PNG *m_instance;
};

#endif
