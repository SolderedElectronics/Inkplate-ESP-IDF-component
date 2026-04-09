#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_

#include "BoardBase.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include <stdint.h>

class BoardCommon : public BoardBase
{
public:
  BoardCommon(uint16_t einkWidth, uint16_t einkHeight,
              uint8_t cleanCycles1, uint8_t cleanCycles0);

  void          setDisplayMode(displayMode_t mode);
  displayMode_t getDisplayMode() { return m_displayMode; }
  void          clearDisplay();
  void          fillDisplay();
  void          writePixelInternal(int16_t x, int16_t y, uint16_t color);
  esp_err_t     display(bool leaveOn = false);
  void          blockGpioPins();
  void          setFullUpdateThreshold(uint16_t numberOfPartialUpdates);
  void          cleanBurnIn(uint8_t cleanCycles, uint16_t cleanDelay);
  double        readBattery();

  esp_err_t     setVCOM(double vcom);
  double        getVCOM();
  double        getStoredVCOM();
  int8_t        readTemperature();

  esp_err_t     sdCardInit();
  esp_err_t     sdCardSleep();
  const char*   getMountPoint();

protected:
  // board-specific display drivers — must be implemented per board
  virtual esp_err_t display1b(bool leaveOn) = 0;
  virtual esp_err_t display3b(bool leaveOn) = 0;
  virtual uint32_t  partialUpdate(bool forced, bool leaveOn) = 0;
  virtual void      clean(uint8_t c, uint8_t rep) = 0;
  virtual void      gpioInit() = 0;
  virtual void      pinsAsOutputs() = 0;
  virtual void      pinsZstate() = 0;
  virtual esp_err_t initBuffers() = 0;
  virtual void      calculateLUTs() = 0;

  // shared low-level helpers
  void      vscanStart();
  void      vscanEnd();
  void      setPanelState(bool state);
  bool      getPanelState();
  esp_err_t pmicBegin();

  uint16_t m_einkWidth;
  uint16_t m_einkHeight;
  uint8_t  m_cleanCycles1;
  uint8_t  m_cleanCycles0;

  displayMode_t m_displayMode = GRAYSCALE;

  uint8_t* m_framebufferColor = nullptr;
  uint8_t* m_framebuffer      = nullptr;
  uint8_t* m_newFramebuffer   = nullptr;
  uint8_t* m_waveformBuffer   = nullptr;

  uint16_t m_partialUpdateLimiter = 10;
  uint16_t m_partialUpdateCounter = 0;
  bool     m_blockPartial = true;
  bool     m_panelState   = false;
};

#endif
