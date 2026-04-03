#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "Shapes/Shapes.h"
#include "GraphicsDefs.h"

class Graphics : public Shapes
{
public:
  Graphics(int16_t w, int16_t h) : Adafruit_GFX(w, h), Shapes(w, h){};

  void setRotation(uint8_t r);
  uint8_t getRotation();

  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void drawTextBox(int16_t x0, int16_t y0, int16_t x1, int16_t x2, const char *text, uint16_t textSize = 1,
           const GFXfont *font = NULL, uint16_t vericalSpacing = 0, bool showBorder = false,
           uint16_t fontSize = 8);

private:
  void writePixel(int16_t x, int16_t y, uint16_t color) override = 0;
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
};


#endif
