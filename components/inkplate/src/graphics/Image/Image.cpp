#include "esp_log.h"

#include "Inkplate.h"
#include "Image.h"

static const char *TAG = "Image";

Image::Image(Inkplate *inkplate) : m_bmp(inkplate)
{
}

/**
 * @brief  Draw an image from a buffer, auto-detecting the format.
 *
 * @param  uint8_t *buf   pointer to image file in memory
 * @param  int x          x position for top-left corner
 * @param  int y          y position for top-left corner
 * @param  bool invert    true to invert colours
 *
 * @return true on success, false if the format is not recognised or supported
 */
bool Image::draw(uint8_t *buf, int x, int y, bool invert)
{
    // BMP: signature "BM"
    if (buf[0] == 0x42 && buf[1] == 0x4D)
        return m_bmp.draw(buf, x, y, invert);

    ESP_LOGE(TAG, "Unrecognised image format");
    return false;
}
