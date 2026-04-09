#ifndef _INKPLATE_6_H_
#define _INKPLATE_6_H_

#include "BoardCommon.h"
#include "pins.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "RTC.h"

#include "../../graphics/GraphicsDefs.h"

#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600

class Inkplate6 : public BoardCommon
{
public:
  Inkplate6();

  uint32_t  partialUpdate(bool forced = false, bool leaveOn = false);
  esp_err_t einkOn() override;
  esp_err_t einkOff() override;

  RTC rtc;

private:
  esp_err_t initBuffers();
  void      calculateLUTs();
  esp_err_t display3b(bool leaveOn);
  esp_err_t display1b(bool leaveOn);
  void      gpioInit();
  void      clean(uint8_t c, uint8_t rep);
  void      pinsAsOutputs();
  void      pinsZstate();

  uint8_t*  m_glut   = nullptr;
  uint8_t*  m_glut2  = nullptr;
  uint32_t* m_pinLUT = nullptr;
};

#endif
