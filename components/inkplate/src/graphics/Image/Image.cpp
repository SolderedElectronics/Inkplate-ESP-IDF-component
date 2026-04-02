#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#include "Inkplate.h"
#include "Image.h"

static const char *TAG = "Image";

Image::Image(Inkplate *inkplate) : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate)
{
}

/**
 * @brief  Draw an image whose size is embedded in the file header (e.g. BMP).
 *
 * @param  buf     Pointer to image file in memory.
 * @param  x       X position for top-left corner.
 * @param  y       Y position for top-left corner.
 * @param  invert  True to invert colours.
 *
 * @return true on success, false if the format is not recognised or supported.
 */
bool Image::draw(uint8_t *buf, int x, int y, bool invert)
{
    // BMP: signature "BM"
    if (buf[0] == 0x42 && buf[1] == 0x4D)
        return m_bmp.draw(buf, x, y, invert);

    ESP_LOGE(TAG, "Unrecognised image format");
    return false;
}

/**
 * @brief  Draw an image where the buffer length must be supplied (e.g. JPEG).
 *
 * @param  buf     Pointer to image file in memory.
 * @param  len     Length of the buffer in bytes.
 * @param  x       X position for top-left corner.
 * @param  y       Y position for top-left corner.
 * @param  invert  True to invert colours.
 *
 * @return true on success, false if the format is not recognised or supported.
 */
bool Image::draw(uint8_t *buf, int32_t len, int x, int y, bool invert)
{
    // JPEG: SOI marker 0xFF 0xD8
    if (buf[0] == 0xFF && buf[1] == 0xD8)
        return m_jpeg.draw(buf, len, x, y, invert);

    // PNG: signature 0x89 0x50 0x4E 0x47
    if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47)
        return m_png.draw(buf, len, x, y, invert);

    // BMP also works here if the caller happens to supply the length
    if (buf[0] == 0x42 && buf[1] == 0x4D)
        return m_bmp.draw(buf, x, y, invert);

    ESP_LOGE(TAG, "Unrecognised image format");
    return false;
}

/**
 * @brief  Download an image from a URL and draw it.
 *
 * @note   HTTP and HTTPS are both supported; the scheme is inferred from the
 *         URL prefix. The downloaded buffer is freed before returning.
 *
 * @param  url     URL of the image to download.
 * @param  x       X position for top-left corner.
 * @param  y       Y position for top-left corner.
 * @param  invert  True to invert colours.
 *
 * @return true on success, false if the download or decode fails.
 */
bool Image::draw(const char *url, int x, int y, bool invert)
{
    int32_t  len = 0;
    uint8_t *buf = nullptr;

    if (strncmp(url, "https://", 8) == 0)
        buf = m_inkplate->wifi.downloadFileHTTPS(url, &len);
    else
        buf = m_inkplate->wifi.downloadFile(url, &len);

    if (!buf)
    {
        ESP_LOGE(TAG, "Failed to download: %s", url);
        return false;
    }

    bool result = draw(buf, len, x, y, invert);
    free(buf);
    return result;
}
