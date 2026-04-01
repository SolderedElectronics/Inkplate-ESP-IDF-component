#ifndef _TPS_H_
#define _TPS_H_

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

#define TPS_I2C_ADDR  0x48
#define TPS_PWR_GOOD  0b11111010

class TPS
{
public:
  esp_err_t begin(i2c_master_bus_handle_t busHandle);
  void      initSequences();
  void      enableRails();
  void      disableRails();
  void      setPowerUpSequence(uint8_t seq);
  void      setPowerDownSequence(uint8_t seq);
  uint8_t   readPowerGood();
  bool      waitPowerGood(bool target);

private:
  void    writeReg(uint8_t reg, uint8_t val);
  uint8_t readReg(uint8_t reg);

  i2c_master_dev_handle_t m_handle = NULL;
};

#endif
