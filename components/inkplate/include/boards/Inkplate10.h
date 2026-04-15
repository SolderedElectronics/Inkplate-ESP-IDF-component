#ifndef INKPLATE_10_H
#define INKPLATE_10_H

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
#define SD_PMOS_PIN  IO_NUM_B2

#define DATA 0x0E8C0030

#define E_INK_WIDTH  1200
#define E_INK_HEIGHT 825

#define INKPLATE10_WAVEFORM1 1

struct waveformData
{
  uint8_t waveformId;
  uint8_t waveform[8][9];
  uint8_t checksum;
};

class Inkplate10 : public BoardCommon
{
public:
  Inkplate10();

  uint32_t  partialUpdate(bool forced = false, bool leaveOn = false);
  esp_err_t einkOn() override;
  esp_err_t einkOff() override;

  RTC rtc;

  esp_err_t setWaveform(uint8_t waveformNumber, bool burnToEEPROM = false);
  esp_err_t getWaveformFromEEPROM(struct waveformData *waveformData);
  esp_err_t changeWaveform(uint8_t *waveform);

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

  uint8_t   calculateChecksum(struct waveformData waveformData);
  esp_err_t burnWaveformToEEPROM(struct waveformData waveformData);

  uint32_t* m_glut   = nullptr;
  uint32_t* m_glut2  = nullptr;
  uint32_t* m_pinLUT = nullptr;
};

#endif
