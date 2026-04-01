#ifndef _INKPLATE_6_H_
#define _INKPLATE_6_H_

#include "BoardBase.h"
#include "PCAL.h"
#include "TPS.h"
#include "pins.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#include "../../graphics/GraphicsDefs.h"

#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600


static const uint8_t waveform3Bit[8][9] =
  {{0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},
   {1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},
   {0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};

class Inkplate6 : public BoardBase
{
public:
  Inkplate6();

  void    setDisplayMode(displayMode_t mode);
  void    writePixelInternal(int16_t x, int16_t y, uint16_t color);
  void    clearDisplay();
  void    fillDisplay();
  void    display(bool leaveOn = false);
  uint32_t partialUpdate(bool forced = false, bool leaveOn = false);
  int     einkOn();
  void    einkOff();
  void    setFullUpdateThreshold(uint16_t numberOfPartialUpdates);

private:
  esp_err_t initBuffers();
  void    calculateLUTs();
  void    display3b(bool leaveOn);
  void    display1b(bool leaveOn);
  void    gpioInit();
  void    clean(uint8_t c, uint8_t rep);
  void    pmicBegin();
  void    vscanStart();
  void    vscanEnd();
  void    pinsAsOutputs();
  void    pinsZstate();
  void    setPanelState(bool state);
  bool    getPanelState();

  displayMode_t           m_displayMode  = GRAYSCALE;

  uint8_t*                m_framebufferColor = nullptr;
  uint8_t*                m_framebuffer      = nullptr;
  // buffer for partial updates
  uint8_t*                m_newFramebuffer   = nullptr;
  // holds the pre-computed waveform data ready to send to the display
  uint8_t*                m_waveformBuffer   = nullptr;

  uint16_t                m_partialUpdateLimiter = 10;
  uint16_t                m_partialUpdateCounter = 0;
  bool                    m_blockPartial = true;

  uint8_t*                m_glut    = nullptr;
  uint8_t*                m_glut2   = nullptr;
  uint32_t*               m_pinLUT  = nullptr;

  bool                    m_panelState = false;
  TPS                     m_tps;

};

#endif
