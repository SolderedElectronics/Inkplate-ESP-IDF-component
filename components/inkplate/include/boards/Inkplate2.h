#ifndef INKPLATE_2_H
#define INKPLATE_2_H

#include "esp_err.h"
#include "esp_rom_sys.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

#include "driver/spi_master.h"

#include "BoardBase.h"
#include "GraphicsDefs.h"

#define EPAPER_RST_PIN  GPIO_NUM_19
#define EPAPER_DC_PIN   GPIO_NUM_33
#define EPAPER_CS_PIN   GPIO_NUM_27
#define EPAPER_BUSY_PIN GPIO_NUM_32
#define EPAPER_CLK      GPIO_NUM_18
#define EPAPER_DIN      GPIO_NUM_23

#define BUSY_TIMEOUT_MS 1000

#define INKPLATE2_WHITE 0
#define INKPLATE2_BLACK 1
#define INKPLATE2_RED   2

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
    {                       \
        int16_t t = a;      \
        a = b;              \
        b = t;              \
    }
#endif

#define E_INK_WIDTH  104
#define E_INK_HEIGHT 212

class Inkplate2 : BoardBase
{
public:
  Inkplate2();

  void      writePixelInternal(int16_t x, int16_t y, uint16_t color);
  esp_err_t display(bool leaveOn = false);
  void      clearDisplay();
  void      fillDisplay();

  void      setDisplayMode(displayMode_t mode) override{}
  esp_err_t einkOn() override{return ESP_OK;}
  esp_err_t einkOff() override{return ESP_OK;}
  void      setFullUpdateThreshold(uint16_t numberOfPartialUpdates) override{}

  uint8_t   m_displayMode = 0;

  uint8_t   *m_framebufferColor = nullptr;
  uint8_t   m_einkHeight = E_INK_HEIGHT;
  uint8_t   m_einkWidth = E_INK_WIDTH;

private:
  bool      waitForEpd(uint32_t timeout);
  void      resetPanel();
  void      sendCommand(uint8_t command);
  void      sendData(uint8_t *data, int n);
  void      sendData(uint8_t data);
  bool      setPanelDeepSleep(bool state);
  
  spi_device_handle_t m_spiDev = nullptr;
};

#endif
