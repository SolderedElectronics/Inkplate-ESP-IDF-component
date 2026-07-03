/**
 * @file GIF.cpp
 * @author Fran Fodor for Soldered
 * @brief GIF image/animation decoder.
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
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

#include <new>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#include "GIF.h"
#include "Inkplate.h"

static const char *TAG = "GIF";

GIF::GIF(Inkplate *inkplate) : m_inkplate(inkplate), m_stopRequested(false) {
  m_gif = (AnimatedGIF *)heap_caps_malloc(sizeof(AnimatedGIF), MALLOC_CAP_SPIRAM);
  new (m_gif) AnimatedGIF();
  m_gif->begin(GIF_PALETTE_RGB888);
}

GIF::~GIF() {
  m_gif->~AnimatedGIF();
  heap_caps_free(m_gif);
}

void GIF::stop() { m_stopRequested = true; }

void GIF::drawCallback(GIFDRAW *pDraw) {
  DrawContext *ctx = (DrawContext *)pDraw->pUser;
  Inkplate *inkplate = ctx->self->m_inkplate;

  const int screenY = ctx->y + pDraw->iY + pDraw->y;
  const uint8_t *pixels = pDraw->pPixels;
  const uint8_t *palette24 = pDraw->pPalette24;

  for (int col = 0; col < pDraw->iWidth; col++) {
    const uint8_t idx = pixels[col];
    if (pDraw->ucHasTransparency && idx == pDraw->ucTransparent)
      continue; // leave whatever is already on screen showing through

    const uint8_t r = palette24[idx * 3 + 0];
    const uint8_t g = palette24[idx * 3 + 1];
    const uint8_t b = palette24[idx * 3 + 2];

    uint8_t val = RGB3BIT(r, g, b);
    if (ctx->invert)
      val ^= 7;
    if (inkplate->getDisplayMode() == BLACK_AND_WHITE)
      val = (~val >> 2) & 1;

    inkplate->drawPixel(ctx->x + pDraw->iX + col, screenY, val);
  }
}

bool GIF::draw(uint8_t *buf, int32_t len, int x, int y, bool invert) {
  if (!m_gif->open(buf, len, drawCallback))
    return false;

  DrawContext ctx{this, x, y, invert};
  int frameDelayMs = 0;
  int rc = m_gif->playFrame(false, &frameDelayMs, &ctx);
  m_gif->close();

  return rc != -1;
}

bool GIF::draw(const char *src, int x, int y, bool invert) {
  int32_t len = 0;
  uint8_t *buf = loadFile(src, &len);
  if (!buf)
    return false;

  bool result = draw(buf, len, x, y, invert);
  free(buf);
  return result;
}

bool GIF::play(uint8_t *buf, int32_t len, int x, int y, bool invert, bool loop,
               uint16_t fullRefreshEveryFrames, bool leaveOn) {
  if (!m_gif->open(buf, len, drawCallback))
    return false;

  DrawContext ctx{this, x, y, invert};
  m_stopRequested = false;
  m_inkplate->setFullUpdateThreshold(fullRefreshEveryFrames);

  int frameDelayMs = 0;
  int rc;
  do {
    rc = m_gif->playFrame(false, &frameDelayMs, &ctx);
    if (rc == -1)
      break;

    m_inkplate->partialUpdate(false, leaveOn);

    if (frameDelayMs > 0)
      vTaskDelay(pdMS_TO_TICKS(frameDelayMs));
  } while ((rc == 1 || loop) && !m_stopRequested);

  m_gif->close();
  return rc != -1;
}

bool GIF::play(const char *src, int x, int y, bool invert, bool loop,
               uint16_t fullRefreshEveryFrames, bool leaveOn) {
  int32_t len = 0;
  uint8_t *buf = loadFile(src, &len);
  if (!buf)
    return false;

  bool result =
      play(buf, len, x, y, invert, loop, fullRefreshEveryFrames, leaveOn);
  free(buf);
  return result;
}

uint8_t *GIF::loadFile(const char *src, int32_t *len) {
  if (strncmp(src, "https://", 8) == 0) {
    uint8_t *buf = m_inkplate->wifi.downloadFileHTTPS(src, len);
    if (!buf)
      ESP_LOGE(TAG, "Failed to download: %s", src);
    return buf;
  }

  if (strncmp(src, "http://", 7) == 0) {
    uint8_t *buf = m_inkplate->wifi.downloadFile(src, len);
    if (!buf)
      ESP_LOGE(TAG, "Failed to download: %s", src);
    return buf;
  }

  char fullPath[256];
  if (src[0] == '/')
    snprintf(fullPath, sizeof(fullPath), "%s", src);
  else
    snprintf(fullPath, sizeof(fullPath), "%s/%s", m_inkplate->getMountPoint(),
             src);

  FILE *f = fopen(fullPath, "rb");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open: %s", fullPath);
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  *len = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t *buf = (uint8_t *)malloc(*len);
  if (!buf) {
    fclose(f);
    ESP_LOGE(TAG, "Out of memory (%ld bytes)", (long)*len);
    return nullptr;
  }

  fread(buf, 1, *len, f);
  fclose(f);
  return buf;
}
