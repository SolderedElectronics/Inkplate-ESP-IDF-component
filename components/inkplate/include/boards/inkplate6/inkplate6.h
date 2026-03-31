#ifndef _INKPLATE_6_H_
#define _INKPLATE_6_H_

#include "i2s.h"
#include "pcal.h"
#include "pins.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600

// TPS65186 PGSTAT register value when all rails are good
#define PWR_GOOD_OK 0b11111010

static const uint8_t LUT2[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95,
                                 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55};
static const uint8_t LUTW[16] = {0xFF, 0xFE, 0xFB, 0xFA, 0xEF, 0xEE, 0xEB, 0xEA,
                                 0xBF, 0xBE, 0xBB, 0xBA, 0xAF, 0xAE, 0xAB, 0xAA};
static const uint8_t LUTB[16] = {0xFF, 0xFD, 0xF7, 0xF5, 0xDF, 0xDD, 0xD7, 0xD5,
                                 0x7F, 0x7D, 0x77, 0x75, 0x5F, 0x5D, 0x57, 0x55};

static const uint8_t pixelMaskLUT[8]  = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
static const uint8_t pixelMaskGLUT[2] = {0xF, 0xF0};

static const uint8_t discharge[16] = {0xFF, 0xFC, 0xF3, 0xF0, 0xCF, 0xCC, 0xC3, 0xC0,
                                      0x3F, 0x3C, 0x33, 0x30, 0x0F, 0x0C, 0x03, 0x00};

static const uint8_t waveform3Bit[8][9] = 
  {{0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},
   {1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},
   {0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};

class Inkplate6 : public I2S
{
public:
  Inkplate6();

  void    begin();
  void    clearDisplay();
  void    fillDisplay();
  void    display3b(bool leaveOn = false);
  int     einkOn();
  void    einkOff();

private:
  void    calculateLUTs();
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

  uint8_t*                m_framebufferColor = nullptr;

  uint8_t                 m_glut[9 * 256];
  uint8_t                 m_glut2[9 * 256];
  uint32_t                m_pinLUT[256];

  bool                    m_panelState   = false;
  i2c_master_dev_handle_t m_tpsHandle    = NULL;
};

#endif
