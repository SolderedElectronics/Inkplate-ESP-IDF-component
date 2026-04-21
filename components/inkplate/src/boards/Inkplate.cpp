#include "Inkplate.h"

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Construct the Inkplate object.
 *
 * @note   Initialises Adafruit_GFX and Graphics with the panel dimensions,
 *         binds the Image helper to this instance, then clears the frame buffer.
 */
Inkplate::Inkplate() : Adafruit_GFX(E_INK_WIDTH, E_INK_HEIGHT), Graphics(E_INK_WIDTH, E_INK_HEIGHT), image(this)
{
  clearDisplay();
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
  setRotation(2);
#endif
}

/**
 * @brief  Draw a single pixel — Adafruit_GFX override.
 *
 * @param  x      X coordinate.
 * @param  y      Y coordinate.
 * @param  color  Pixel value (0-7 in grayscale mode, 0-1 in B&W mode).
 */
void Inkplate::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  writePixel(x, y, color);
}

/**
 * @brief  Return the current display rotation (0-3, matching Adafruit_GFX convention).
 *
 * @return uint8_t  Rotation index.
 */
uint8_t Inkplate::getRotation()
{
  return rotation;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Write a single pixel to the frame buffer.
 *
 * @param  x      X coordinate.
 * @param  y      Y coordinate.
 * @param  color  Pixel value.
 */
void Inkplate::writePixel(int16_t x, int16_t y, uint16_t color)
{
  writePixelInternal(x, y, color);
}
