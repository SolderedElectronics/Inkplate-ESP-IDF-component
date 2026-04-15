#ifndef INKPLATE_5_H
#define INKPLATE_5_H

#include "esp_err.h"
#include "esp_rom_sys.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

#include "BoardCommon.h"
#include "GraphicsDefs.h"

#include "PCAL.h"
#include "RTC.h"

extern PCAL expander1;

#define IO_INT_ADDR  0x20

// pin on the internal io expander which controls MOSFET for turning on and off the SD card
#define SD_PMOS_PIN  IO_NUM_B2

#define E_INK_WIDTH  1280
#define E_INK_HEIGHT 720

class Inkplate5 : public BoardCommon
{
public:
  Inkplate5();

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

  uint32_t* m_glut   = nullptr;
  uint32_t* m_glut2  = nullptr;
  uint32_t* m_pinLUT = nullptr;
};

#endif
