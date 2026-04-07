#ifndef _INKPLATE_10_H_
#define _INKPLATE_10_H_

#include "BoardCommon.h"
#include "pins.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#include "../../graphics/GraphicsDefs.h"

#define E_INK_WIDTH  1200
#define E_INK_HEIGHT 825

class Inkplate10 : public BoardCommon
{
public:
  Inkplate10();

  uint32_t partialUpdate(bool forced = false, bool leaveOn = false);

private:
  esp_err_t initBuffers();
  void      calculateLUTs();
  esp_err_t display3b(bool leaveOn);
  esp_err_t display1b(bool leaveOn);
  void      hscanStart(uint32_t data);
  void      gpioInit();
  void      clean(uint8_t c, uint8_t rep);
  void      pinsAsOutputs();
  void      pinsZstate();
  void      einkOnBoardInit();
  void      einkOffClearPins();

  uint32_t* m_glut   = nullptr;
  uint32_t* m_glut2  = nullptr;
  uint32_t* m_pinLUT = nullptr;
};

#endif
