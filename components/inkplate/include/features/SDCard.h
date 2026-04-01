#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "PCAL.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"

// SD power switch on IO expander
#define SD_PMOS_PIN IO_NUM_B2

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

  esp_err_t   sdCardInit();
  esp_err_t   sdCardSleep();
  const char *getMountPoint();

private:
  PCAL         &m_expander;
  sdmmc_card_t *m_card = nullptr;
};

#endif
