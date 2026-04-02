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
 * @brief  Draw an image from a URL or an SD card path.
 *
 * @note   The source is selected automatically from the @p src prefix:
 *           - "https://" → HTTPS download
 *           - "http://"  → HTTP download
 *           - anything else → SD card file; the mount point is prepended
 *             automatically if the path does not start with '/'.
 *         The SD card must already be mounted when passing a file path.
 *
 * @param  src     URL or SD card file path.
 * @param  x       X position for top-left corner.
 * @param  y       Y position for top-left corner.
 * @param  invert  True to invert colours.
 *
 * @return true on success, false if the download/read or decode fails.
 */
bool Image::draw(const char *src, int x, int y, bool invert)
{
    int32_t  len = 0;
    uint8_t *buf = nullptr;

    if (strncmp(src, "https://", 8) == 0)
    {
        buf = m_inkplate->wifi.downloadFileHTTPS(src, &len);
        if (!buf)
        {
            ESP_LOGE(TAG, "Failed to download: %s", src);
            return false;
        }
    }
    else if (strncmp(src, "http://", 7) == 0)
    {
        buf = m_inkplate->wifi.downloadFile(src, &len);
        if (!buf)
        {
            ESP_LOGE(TAG, "Failed to download: %s", src);
            return false;
        }
    }
    else
    {
        // SD card path — prepend mount point if not absolute
        char fullPath[256];
        if (src[0] == '/')
            snprintf(fullPath, sizeof(fullPath), "%s", src);
        else
            snprintf(fullPath, sizeof(fullPath), "%s/%s", m_inkplate->getMountPoint(), src);

        FILE *f = fopen(fullPath, "rb");
        if (!f)
        {
            ESP_LOGE(TAG, "Failed to open: %s", fullPath);
            return false;
        }

        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek(f, 0, SEEK_SET);

        buf = (uint8_t *)malloc(len);
        if (!buf)
        {
            fclose(f);
            ESP_LOGE(TAG, "Out of memory (%ld bytes)", len);
            return false;
        }

        fread(buf, 1, len, f);
        fclose(f);
    }

    bool result = draw(buf, len, x, y, invert);
    free(buf);
    return result;
}
