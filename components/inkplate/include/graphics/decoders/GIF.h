/**
 * @file GIF.h
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

#pragma once

#include "stdlib.h"

#include "AnimatedGIF.h"

class Inkplate;

/**
 * @brief Class for decoding and playing GIF images/animations.
 *
 * @note Every pixel is converted with a fixed luminance threshold (RGB3BIT,
 *       no error-diffusion dithering) - dithering noise recomputed per frame
 *       reads as flicker between frames, so it is intentionally left out of
 *       the animation path.
 */
class GIF {
public:
  /**
   * @brief Construct a new GIF object.
   *
   * @param inkplate inkplate instance.
   */
  GIF(Inkplate *inkplate);

  /**
   * @brief Destroy the GIF object, freeing the PSRAM-backed AnimatedGIF
   * decoder state.
   */
  ~GIF();

  /**
   * @brief Draws only the first frame of a GIF held in a raw buffer, e.g. to
   * use a GIF as a static "poster" image.
   *
   * @param buf pointer to the GIF file buffer.
   * @param len length of the buffer in bytes.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param invert true to invert pixel values.
   * @return bool true on success, false on open/decode error.
   *
   * @note caller is responsible for calling display()/partialUpdate()
   * afterwards, same convention as Image::draw().
   */
  bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false);

  /**
   * @brief Draws only the first frame of a GIF loaded from a URL or an SD
   * card path.
   *
   * @param src URL or file path of the GIF.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param invert true to invert pixel values.
   * @return bool true on success, false on download, file, memory, or decode
   * error.
   *
   * @note The source is selected by prefix:
   *       - "https://" -> HTTPS download
   *       - "http://"  -> HTTP download
   *       - anything else -> SD card file (mount point prepended if path is
   * relative)
   */
  bool draw(const char *src, int x, int y, bool invert = false);

  /**
   * @brief Plays every frame of a GIF held in a raw buffer, blocking until
   * finished (or forever if loop is true).
   *
   * @param buf pointer to the GIF file buffer.
   * @param len length of the buffer in bytes.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param invert true to invert pixel values.
   * @param loop true to keep playing forever until stop() is called.
   * @param fullRefreshEveryFrames forwarded to
   * Inkplate::setFullUpdateThreshold(): the driver forces a full refresh
   * every N partialUpdate() calls to clear partial-update ghosting. Pass 0 to
   * disable forced full refreshes.
   * @param leaveOn forwarded to Inkplate::partialUpdate(): if true, panel
   * power is left on between frames instead of being switched off and back
   * on for every single frame.
   * @return bool true if playback ran to completion/was stopped cleanly,
   * false on open/decode error.
   *
   * @note e-ink partial refresh time (hundreds of ms) will normally dominate
   * over the GIF's own frame delay, so real playback speed is limited by the
   * panel, not by the file.
   */
  bool play(uint8_t *buf, int32_t len, int x, int y, bool invert = false,
            bool loop = true, uint16_t fullRefreshEveryFrames = 60,
            bool leaveOn = true);

  /**
   * @brief Plays every frame of a GIF loaded from a URL or an SD card path.
   * See play(uint8_t*, int32_t, int, int, bool, bool, uint16_t, bool).
   *
   * @param src URL or file path of the GIF.
   * @return bool true if playback ran to completion/was stopped cleanly,
   * false on download, file, memory, or decode error.
   */
  bool play(const char *src, int x, int y, bool invert = false,
            bool loop = true, uint16_t fullRefreshEveryFrames = 60,
            bool leaveOn = true);

  /**
   * @brief Call from a button press / ISR-set flag to break out of a running
   * play() call.
   */
  void stop();

private:
  /**
   * @brief Per-draw context passed to the AnimatedGIF callback via pUser.
   */
  struct DrawContext {
    GIF *self;
    int x, y;
    bool invert;
  };

  /**
   * @brief Loads a whole file from a URL or SD card path into a malloc'd
   * buffer, mirroring Image::draw(const char*, ...)'s source resolution.
   *
   * @param src URL or file path.
   * @param len output: length of the returned buffer in bytes.
   * @return uint8_t* malloc'd buffer, or nullptr on error. Caller must free
   * it.
   */
  uint8_t *loadFile(const char *src, int32_t *len);

  /**
   * @brief AnimatedGIF draw callback - called once per decoded scanline.
   * Converts the RGB888 palette pixels of that line to the panel's native
   * depth using a fixed luminance threshold and writes them straight into
   * the Inkplate framebuffer.
   *
   * @param pDraw decoded scanline description.
   */
  static void drawCallback(GIFDRAW *pDraw);

  Inkplate *m_inkplate;
  // Heap-allocated in PSRAM rather than embedded by value: GIFIMAGE's LZW
  // dictionaries/palettes/line buffers total ~24KB, which would otherwise be
  // added directly to sizeof(Inkplate) and overflow the caller's stack when
  // Inkplate is declared as a local variable (the convention used everywhere
  // in this codebase, e.g. `Inkplate display;` inside app_main()).
  AnimatedGIF *m_gif;
  volatile bool m_stopRequested;
};
