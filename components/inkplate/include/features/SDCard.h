#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "PCAL.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"

#if CONFIG_INKPLATE_BOARD_INKPLATE6
  #include "inkplate6/pins.h"
  #define INKPLATE_BOARD_CLASS Inkplate6
#elif CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  #include "inkplate6color/pins.h"
  #define INKPLATE_BOARD_CLASS Inkplate6Color
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "inkplate10/pins.h"
  #define INKPLATE_BOARD_CLASS Inkplate10
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
  #include "inkplate5/pins.h"
  #define INKPLATE_BOARD_CLASS Inkplate5
#elif CONFIG_INKPLATE_BOARD_INKPLATE4
  #include "inkplate4/pins.h"
  #define INKPLATE_BOARD_CLASS Inkplate4
#else
  #error "No Inkplate board selected. Choose a board in menuconfig -> Inkplate Board."
#endif

// SPI pin numbers (GPIO)
#define SD_SCK  14
#define SD_MISO 12
#define SD_MOSI 13
#define SD_CS   15

#define SD_MOUNT_POINT "/sdcard"

class SDCard
{
public:
  SDCard(PCAL &expander);

  esp_err_t    sdCardInit();
  esp_err_t    sdCardSleep();
  const char  *getMountPoint();

private:
  PCAL         &m_expander;
  sdmmc_card_t *m_card = nullptr;
};

#endif
