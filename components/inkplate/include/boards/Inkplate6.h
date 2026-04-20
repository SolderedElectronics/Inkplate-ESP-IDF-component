#ifndef INKPLATE_6_H
#define INKPLATE_6_H

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
#define IO_EXT_ADDR  0x21

// pin on the internal io expander which controls MOSFET for turning on and off the SD card

#if CONFIG_INKPLATE_BOARD_INKPLATE6
#define SD_PMOS_PIN  IO_NUM_B2
#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600
static const uint8_t waveform3Bit[8][9] =
{{0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},
{1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},
{0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};
#elif CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#include "Frontlight.h"
#include "Touchscreen.h"
#define SD_PMOS_PIN  IO_NUM_B5
#define E_INK_WIDTH  1024
#define E_INK_HEIGHT 758
#define FRONTLIGHT_EN IO_NUM_B3

#define TOUCHSCREEN_EN          IO_NUM_B4
#define TOUCHSCREEN_RST         IO_NUM_B2
#define TOUCHSCREEN_INT         GPIO_NUM_36
#define TOUCHSCREEN_IO_EXPANDER IO_INT_ADDR
#define TOUCHSCREEN_IO_REGS     ioRegsInt
static const uint8_t waveform3Bit[8][9] =
   {{0, 0, 0, 0, 0, 1, 1, 1, 0}, {0, 0, 1, 2, 1, 1, 2, 1, 0}, {0, 1, 1, 2, 1, 1, 1, 2, 0},
    {1, 1, 1, 2, 2, 1, 1, 2, 0}, {1, 1, 1, 2, 1, 2, 1, 2, 0}, {0, 1, 1, 2, 1, 2, 1, 2, 0},
    {1, 2, 1, 1, 2, 2, 1, 2, 0}, {0, 0, 0, 0, 0, 0, 0, 2, 0}};
#endif

class Inkplate6 : public BoardCommon
{
public:
  Inkplate6();

  uint32_t  partialUpdate(bool forced = false, bool leaveOn = false);
  esp_err_t einkOn() override;
  esp_err_t einkOff() override;

  RTC rtc;
  #if CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
  Frontlight frontlight;
  Touch touchscreen;
  #endif

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
