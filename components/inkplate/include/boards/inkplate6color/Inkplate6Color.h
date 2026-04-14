#ifndef _INKPLATE_6_COLOR_H_
#define _INKPLATE_6_COLOR_H_

#include "BoardCommon.h"
#include "pins.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "RTC.h"

#include "driver/spi_master.h"

#include "../../graphics/GraphicsDefs.h"

class Inkplate6Color : public BoardCommon
{
public:
  Inkplate6Color();
  
  uint32_t  partialUpdate(bool forced = false, bool leaveOn = false) {return 0;};
  esp_err_t einkOn() override;
  esp_err_t einkOff() override;

  RTC       rtc;

private:
  esp_err_t initBuffers();
  esp_err_t display3b(bool leaveOn);
  bool      waitForEpd(uint32_t timeout);
  void      resetPanel();
  void      sendCommand(uint8_t command);
  void      sendData(uint8_t *data, int n);
  void      sendData(uint8_t data);
  bool      setPanelDeepSleep(bool state);

  void      calculateLUTs() {return;};
  esp_err_t display1b(bool leaveOn) {return ESP_OK;};
  void      gpioInit() {return;};
  void      clean(uint8_t c, uint8_t rep) {return;};
  void      pinsAsOutputs() {return;};
  void      pinsZstate() {return;};

  spi_device_handle_t m_spiDev = nullptr;
};

#endif
