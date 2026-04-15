#ifndef BOARD_COMMON_H
#define BOARD_COMMON_H

#include "BoardBase.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include <stdint.h>

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE10) || \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE6)  || \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE5)  || \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE4)
// enables the GPIO0/CL level shifter
#define GPIO0_ENABLE IO_NUM_B0

#define WAKEUP       IO_NUM_A3
#define WAKEUP_SET   do { expander1.setLevel(WAKEUP, 1, true); } while(0)
#define WAKEUP_CLEAR do { expander1.setLevel(WAKEUP, 0, true); } while(0)

#define PWRUP        IO_NUM_A4
#define PWRUP_SET    do { expander1.setLevel(PWRUP, 1, true); } while(0)
#define PWRUP_CLEAR  do { expander1.setLevel(PWRUP, 0, true); } while(0)

#define VCOM         IO_NUM_A5
#define VCOM_SET     do { expander1.setLevel(VCOM, 1, true); } while(0)
#define VCOM_CLEAR   do { expander1.setLevel(VCOM, 0, true); } while(0)

#define OE           IO_NUM_A0
#define OE_SET       do { expander1.setLevel(OE, 1, true); } while(0)
#define OE_CLEAR     do { expander1.setLevel(OE, 0, true); } while(0)

#define GMOD         IO_NUM_A1
#define GMOD_SET     do { expander1.setLevel(GMOD, 1, true); } while(0)
#define GMOD_CLEAR   do { expander1.setLevel(GMOD, 0, true); } while(0)

#define SPV          IO_NUM_A2
#define SPV_SET      do { expander1.setLevel(SPV, 1, true); } while(0)
#define SPV_CLEAR    do { expander1.setLevel(SPV, 0, true); } while(0)

#define CL           0x01
#define CL_SET       do { GPIO.out_w1ts = CL; } while(0)
#define CL_CLEAR     do { GPIO.out_w1tc = CL; } while(0)

#define CKV          0x01
#define CKV_SET      do { GPIO.out1_w1ts.val = CKV; } while(0)
#define CKV_CLEAR    do { GPIO.out1_w1tc.val = CKV; } while(0)

#define SPH          0x02
#define SPH_SET      do { GPIO.out1_w1ts.val = SPH; } while(0)
#define SPH_CLEAR    do { GPIO.out1_w1tc.val = SPH; } while(0)

#define LE           0x04
#define LE_SET       do { GPIO.out_w1ts = LE; } while(0)
#define LE_CLEAR     do { GPIO.out_w1tc = LE; } while(0)
#endif

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
