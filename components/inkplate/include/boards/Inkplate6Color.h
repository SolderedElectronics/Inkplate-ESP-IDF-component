#ifndef INKPLATE_6_COLOR_H
#define INKPLATE_6_COLOR_H

#include "esp_err.h"
#include "esp_rom_sys.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

#include "BoardCommon.h"
#include "GraphicsDefs.h"

#include "SPI.h"
#include "PCAL.h"
#include "RTC.h"

#define IO_INT_ADDR 0x20

extern PCAL expander1;

// Pin on the internal io expander which controls MOSFET for turning on and off the SD card
#define SD_PMOS_PIN IO_NUM_B2 // 10

// Connections between ESP32 and color Epaper
#define EPAPER_RST_PIN  GPIO_NUM_19
#define EPAPER_DC_PIN   GPIO_NUM_33
#define EPAPER_CS_PIN   GPIO_NUM_27
#define EPAPER_BUSY_PIN GPIO_NUM_32
#define EPAPER_CLK      GPIO_NUM_18
#define EPAPER_DIN      GPIO_NUM_23

// Timeout for init of epaper (1.5 sec in this case)
#define INIT_TIMEOUT 1500

// Epaper registers
#define PANEL_SET_REGISTER          0x00
#define POWER_SET_REGISTER          0x01
#define POWER_OFF_SEQ_SET_REGISTER  0x03
#define POWER_OFF_REGISTER          0x04
#define BOOSTER_SOFTSTART_REGISTER  0x06
#define DEEP_SLEEP_REGISTER         0x07
#define DATA_START_TRANS_REGISTER   0x10
#define DATA_STOP_REGISTER          0x11
#define DISPLAY_REF_REGISTER        0x12
#define IMAGE_PROCESS_REGISTER      0x13
#define PLL_CONTROL_REGISTER        0x30
#define TEMP_SENSOR_REGISTER        0x40
#define TEMP_SENSOR_EN_REGISTER     0x41
#define TEMP_SENSOR_WR_REGISTER     0x42
#define TEMP_SENSOR_RD_REGISTER     0x43
#define VCOM_DATA_INTERVAL_REGISTER 0x50
#define LOW_POWER_DETECT_REGISTER   0x51
#define RESOLUTION_SET_REGISTER     0x61
#define STATUS_REGISTER             0x71
#define VCOM_VALUE_REGISTER         0x81
#define VCM_DC_SET_REGISTER         0x02

// Epaper resolution and colors
#define E_INK_WIDTH     600
#define E_INK_HEIGHT    448
#define INKPLATE_BLACK  0b00000000
#define INKPLATE_WHITE  0b00000001
#define INKPLATE_GREEN  0b00000010
#define INKPLATE_BLUE   0b00000011
#define INKPLATE_RED    0b00000100
#define INKPLATE_YELLOW 0b00000101
#define INKPLATE_ORANGE 0b00000110

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

  void      setIOExpanderForLowPower();

  void      calculateLUTs() {return;};
  esp_err_t display1b(bool leaveOn) {return ESP_OK;};
  void      gpioInit() {return;};
  void      clean(uint8_t c, uint8_t rep) {return;};
  void      pinsAsOutputs() {return;};
  void      pinsZstate() {return;};

  SPI       m_spi;
};

#endif
