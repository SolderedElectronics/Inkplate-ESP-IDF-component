#ifndef _INKPLATE_4_H_
#define _INKPLATE_4_H_

#include "BoardCommon.h"
#include "pins.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#include "../../graphics/GraphicsDefs.h"

#include "RTC.h"
#include "APDS9960.h"
#include "BQ27441.h"
#include "LSM6DS3.h"
#include "BME680.h"

#include "Frontlight.h"

#define E_INK_WIDTH  600
#define E_INK_HEIGHT 600

class Inkplate4 : public BoardCommon
{
public:
  Inkplate4();

  uint32_t  partialUpdate(bool forced = false, bool leaveOn = false);
  esp_err_t einkOn() override;
  esp_err_t einkOff() override;

  RTC      rtc;
  APDS9960 apds;
  BQ27441  bq;
  LSM6DS3  lsm;
  BME680   bme;

  Frontlight frontlight;

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

  uint32_t* m_glut   = nullptr;
  uint32_t* m_glut2  = nullptr;
  uint32_t* m_pinLUT = nullptr;
};

#endif
