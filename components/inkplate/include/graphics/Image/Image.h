#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "BMP/BMP.h"
#include "JPEG/JPEG.h"
#include "PNG/PNG.h"

class Inkplate;

/**
 * @brief Format-agnostic image decoder.
 *
 * @note  Format is detected automatically from the magic bytes in the buffer.
 *        Use the two-argument length overload for formats (e.g. JPEG) whose
 *        size is not embedded in the file header.
 */
class Image
{
public:
    Image(Inkplate *inkplate);

    /**
     * @brief  Draw an image whose size is embedded in the file (e.g. BMP).
     *
     * @param  buf     Pointer to the image data in memory.
     * @param  x       X position of the top-left corner on the display.
     * @param  y       Y position of the top-left corner on the display.
     * @param  invert  True to invert colours.
     *
     * @return true on success, false if the format is not recognised or not supported.
     */
    bool draw(uint8_t *buf, int x, int y, bool invert = false);

    /**
     * @brief  Draw an image where the buffer length must be supplied (e.g. JPEG).
     *
     * @param  buf     Pointer to the image data in memory.
     * @param  len     Length of the buffer in bytes.
     * @param  x       X position of the top-left corner on the display.
     * @param  y       Y position of the top-left corner on the display.
     * @param  invert  True to invert colours.
     *
     * @return true on success, false if the format is not recognised or not supported.
     */
    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);

    /**
     * @brief  Download an image from a URL and draw it.
     *
     * @note   Format is auto-detected from the downloaded data. HTTP and HTTPS
     *         are both supported; the scheme is inferred from the URL prefix.
     *
     * @param  url     URL of the image to download.
     * @param  x       X position of the top-left corner on the display.
     * @param  y       Y position of the top-left corner on the display.
     * @param  invert  True to invert colours.
     *
     * @return true on success, false if the download or decode fails.
     */
    bool draw(const char *url, int x, int y, bool invert = false);

private:
    Inkplate *m_inkplate;
    BMP       m_bmp;
    JPEG      m_jpeg;
    PNG       m_png;
};

#endif
