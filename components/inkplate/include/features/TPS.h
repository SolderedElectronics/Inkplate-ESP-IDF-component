#ifndef _TPS_H_
#define _TPS_H_

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

#define TPS_I2C_ADDR  0x48

#define TPS_PWR_GOOD  0b11111010
#define TPS_PWRUP_SEQ 0b11100100
#define TPS_PWRDN_SEQ 0b00011011

class TPS
{
public:
  TPS(i2c_master_bus_handle_t busHandle);

  esp_err_t initSequences();
  esp_err_t enableRails();
  esp_err_t disableRails();
  esp_err_t setPowerUpSequence(uint8_t seq);
  esp_err_t setPowerDownSequence(uint8_t seq);
  uint8_t   readPowerGood();
  bool      waitPowerGood(bool target);

private:
  esp_err_t writeReg(uint8_t reg, uint8_t val);
  uint8_t   readReg(uint8_t reg);

  i2c_master_dev_handle_t m_handle = NULL;
};

#endif
