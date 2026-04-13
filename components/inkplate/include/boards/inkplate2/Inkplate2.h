#ifndef _INKPLATE_2_H_
#define _INKPLATE_2_H_

#include "pins.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#include "driver/spi_master.h"

#include "../../graphics/GraphicsDefs.h"

#define E_INK_WIDTH  104
#define E_INK_HEIGHT 212

class Inkplate2
{
public:
  Inkplate2();

  void writePixelInternal(int16_t x, int16_t y, uint16_t color);
  esp_err_t display(bool leaveOn = false);
  void setDisplayMode(uint8_t displayMode);
  void clearDisplay();
  void fillDisplay();

  //ImageColor image;

  uint8_t m_displayMode = 0;

  uint8_t *m_framebufferColor = nullptr;
  uint8_t m_einkHeight = E_INK_HEIGHT;
  uint8_t m_einkWidth = E_INK_WIDTH;

private:
  bool waitForEpd(uint32_t timeout);
  void resetPanel();
  void sendCommand(uint8_t command);
  void sendData(uint8_t *data, int n);
  void sendData(uint8_t data);
  bool setPanelDeepSleep(bool state);
  spi_device_handle_t m_spiDev = nullptr;
};

#endif
