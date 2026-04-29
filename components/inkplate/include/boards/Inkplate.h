/**
 * @file Inkplate.h
 * @author Fran Fodor for Soldered
 * @brief Inkplate fuctions for Adafruit_GFX overrides.
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

#include "WiFi.h"
#include "graphics/Graphics.h"
#include "sdkconfig.h"

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2) ||                                \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) ||                           \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#define COLOR_IMAGE
#endif

#ifdef COLOR_IMAGE
#include "ImageColor.h"
#else
#include "Image.h"
#endif

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6) ||                                \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
#include "Inkplate6.h"
#define INKPLATE_BOARD_CLASS Inkplate6
#elif CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#include "Inkplate6color.h"
#define INKPLATE_BOARD_CLASS Inkplate6Color
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
#include "Inkplate10.h"
#define INKPLATE_BOARD_CLASS Inkplate10
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#include "Inkplate13.h"
#define INKPLATE_BOARD_CLASS Inkplate13
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
#include "Inkplate5.h"
#define INKPLATE_BOARD_CLASS Inkplate5
#elif CONFIG_INKPLATE_BOARD_INKPLATE4
#include "Inkplate4.h"
#define INKPLATE_BOARD_CLASS Inkplate4
#elif CONFIG_INKPLATE_BOARD_INKPLATE2
#include "Inkplate2.h"
#define INKPLATE_BOARD_CLASS Inkplate2
#else
#error                                                                         \
    "No Inkplate board selected. Choose a board in menuconfig -> Inkplate Board."
#endif

/**
 * @brief Class for inkplate overrides.
 *
 */
class Inkplate : public Graphics, public INKPLATE_BOARD_CLASS {
public:
  /**
   * @brief Construct a new Inkplate object.
   *
   */
  Inkplate();

#ifndef COLOR_IMAGE
  /**
   * @brief Copies the framebuffer to partial for deepsleep restore.
   *
   */
  void preloadScreen();
#endif
  /**
   * @brief Draw a single pixel — Adafruit_GFX override.
   *
   * @param x x coordinate.
   * @param y y coordinate.
   * @param color pixel value (0-7 in grayscale mode, 0-1 in B&W mode).
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /**
   * @brief Return the current display rotation (0-3, matching Adafruit_GFX
   * convention).
   *
   * @return uint8_t rotation index.
   */
  uint8_t getRotation() override;

#ifdef COLOR_IMAGE
  ImageColor image;
#else
  Image image;
#endif

  WiFi wifi;

private:
  /**
   * @brief Write a single pixel to the frame buffer.
   *
   * @param x x coordinate.
   * @param y y coordinate.
   * @param color pixel value.
   */
  void writePixel(int16_t x, int16_t y, uint16_t color);
};