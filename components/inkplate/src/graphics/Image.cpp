/**
 * @file   Image.cpp
 * @author Fran Fodor for Soldered
 * @brief  Drawing black and white/grayscale images.
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "string.h"
#include "esp_log.h"

#include "Inkplate.h"
#include "Image.h"

static const char *TAG = "Image";

/* -------------------------------------------------------------------------- */
/*                              Public functions                               */
/* -------------------------------------------------------------------------- */

Image::Image(Inkplate *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate)
{
    m_dither          = false;
    m_ditherBuffer[0] = m_ditherBuffer[1] = nullptr;
}

uint8_t Image::getDitheredPixel(uint32_t px, int i, int w, bool paletted)
{
    /* paletted: shared decoder already unpacked palette entry to luminance */

    if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
        px = (uint16_t)px >> 1;

    uint16_t sum     = (uint16_t)m_ditherBuffer[0][i] + (uint16_t)px;
    uint8_t oldPixel = sum > 0xFF ? 0xFF : (uint8_t)sum;

    uint8_t newPixel   = oldPixel & (m_inkplate->getDisplayMode() == BLACK_AND_WHITE ? 0x80 : 0xE0);
    uint8_t quantError = oldPixel - newPixel;

    /* distribute quantisation error: right=7/16, below-left=3/16, below=5/16, below-right=1/16 */
    m_ditherBuffer[1][i] += (quantError * 5) >> 4;
    if (i != w - 1)
    {
        m_ditherBuffer[0][i + 1] += (quantError * 7) >> 4;
        m_ditherBuffer[1][i + 1] += (quantError * 1) >> 4;
    }
    if (i != 0)
        m_ditherBuffer[1][i - 1] += (quantError * 3) >> 4;

    return newPixel >> 5;
}

void Image::ditherSwap(int w)
{
    for (int i = 0; i < w; ++i)
    {
        m_ditherBuffer[0][i] = m_ditherBuffer[1][i];
        m_ditherBuffer[1][i] = 0;
    }
}

bool Image::draw(uint8_t *buf, int x, int y, bool dither, bool invert)
{
    m_dither = dither;
    if (dither) beginDither();

    bool result = false;
    if (buf[0] == 0x42 && buf[1] == 0x4D) /* BMP magic bytes */
        result = m_bmp.draw(buf, x, y, dither, invert);
    else
        ESP_LOGE(TAG, "Unrecognised image format");

    if (dither) endDither();
    m_dither = false;
    return result;
}

bool Image::draw(uint8_t *buf, int32_t len, int x, int y, bool dither, bool invert)
{
    m_dither = dither;
    if (dither) beginDither();

    bool result = false;
    if (buf[0] == 0xFF && buf[1] == 0xD8)                                          /* JPEG magic bytes */
        result = m_jpeg.draw(buf, len, x, y, dither, invert);
    else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47) /* PNG magic bytes */
        result = m_png.draw(buf, len, x, y, dither, invert);
    else if (buf[0] == 0x42 && buf[1] == 0x4D)                                     /* BMP magic bytes */
        result = m_bmp.draw(buf, x, y, dither, invert);
    else
        ESP_LOGE(TAG, "Unrecognised image format");

    if (dither) endDither();
    m_dither = false;
    return result;
}

bool Image::draw(const char *src, int x, int y, bool dither, bool invert)
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

    bool result = draw(buf, len, x, y, dither, invert);
    free(buf);
    return result;
}

bool Image::draw(const uint8_t *buf, int w, int h, int x, int y, bool dither, bool invert)
{
    m_dither = dither;
    if (dither) beginDither();

    for (int row = 0; row < h; ++row)
    {
        const int bytesPerRow = (w + 3) / 4; /* 4 pixels packed per byte */
        for (int col = 0; col < w; ++col)
        {
            int byteIdx = row * bytesPerRow + col / 4;
            int shift   = 6 - (col % 4) * 2;
            uint8_t px  = (buf[byteIdx] >> shift) & 0x03;

            /* map 2bpp value to 8-bit luminance: 0→0, 1→85, 2→170, 3→255 */
            uint8_t luma = px * 85;
            if (invert) luma = 255 - luma;

            uint8_t out;
            if (dither)
                out = getDitheredPixel(luma, col, w, false);
            else
                out = luma >> 5; /* convert to 3-bit greyscale */

            m_inkplate->drawPixel(x + col, y + row, out);
        }
        if (dither) ditherSwap(w);
    }

    if (dither) endDither();
    m_dither = false;
    return true;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void Image::beginDither()
{
    m_ditherBuffer[0] = (uint8_t *)calloc(BMP_MAX_WIDTH + 2, 1);
    m_ditherBuffer[1] = (uint8_t *)calloc(BMP_MAX_WIDTH + 2, 1);
}

void Image::endDither()
{
    free(m_ditherBuffer[0]);
    free(m_ditherBuffer[1]);
    m_ditherBuffer[0] = m_ditherBuffer[1] = nullptr;
}