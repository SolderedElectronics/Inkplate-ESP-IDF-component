#ifndef SDCARD_H
#define SDCARD_H

#include "PCAL.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"

// SPI pin numbers (GPIO)
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#define SD_MISO GPIO_NUM_13
#define SD_MOSI GPIO_NUM_11
#define SD_SCK  GPIO_NUM_12
#define SD_CS   GPIO_NUM_10
#else
#define SD_MISO GPIO_NUM_12
#define SD_MOSI GPIO_NUM_13
#define SD_SCK  GPIO_NUM_14
#define SD_CS   GPIO_NUM_15
#endif

#define SD_MOUNT_POINT "/sdcard"

class SDCard
{
public:
  SDCard(PCAL &expander, IOPin_t pin);

  esp_err_t    sdCardInit();
  esp_err_t    sdCardSleep();
  const char  *getMountPoint();

private:
  PCAL         &m_expander;
  sdmmc_card_t *m_card = nullptr;
  IOPin_t       m_pin;
};

#endif
