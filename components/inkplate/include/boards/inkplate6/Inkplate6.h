#ifndef _INKPLATE_6_H_
#define _INKPLATE_6_H_

#include "BoardBase.h"
#include "PCAL.h"
#include "pins.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#include "../../graphics/GraphicsDefs.h"

#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600

// TPS65186 PGSTAT register value when all rails are good
#define PWR_GOOD_OK 0b11111010

static const uint8_t waveform3Bit[8][9] =
  {{0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},
   {1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},
   {0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};

class Inkplate6 : public BoardBase
{
public:
  Inkplate6();

  void    begin();
  void    setDisplayMode(displayMode_t mode);
  void    writePixelInternal(int16_t x, int16_t y, uint16_t color);
  void    clearDisplay();
  void    fillDisplay();
  void    display(bool leaveOn = false);
  uint32_t partialUpdate(bool forced = false, bool leaveOn = false);
  int     einkOn();
  void    einkOff();

private:
  void    calculateLUTs();
  void    display3b(bool leaveOn);
  void    display1b(bool leaveOn);
  void    gpioInit();
  void    clean(uint8_t c, uint8_t rep);
  void    pmicBegin();
  uint8_t readPowerGood();
  bool    waitPowerGood(bool target);
  void    vscanStart();
  void    vscanEnd();
  void    pinsAsOutputs();
  void    pinsZstate();
  void    setPanelState(bool state);
  bool    getPanelState();

  displayMode_t           m_displayMode  = GRAYSCALE;

  uint8_t*                m_framebufferColor = nullptr;
  uint8_t*                m_framebuffer      = nullptr;
  uint8_t*                m_newFramebuffer   = nullptr;
  uint8_t*                m_pBuffer          = nullptr;

  uint16_t                m_partialUpdateLimiter = 10;
  uint16_t                m_partialUpdateCounter = 0;

  uint8_t                 m_glut[9 * 256];
  uint8_t                 m_glut2[9 * 256];
  uint32_t                m_pinLUT[256];

  bool                    m_panelState   = false;
  i2c_master_dev_handle_t m_tpsHandle    = NULL;

};

#endif
