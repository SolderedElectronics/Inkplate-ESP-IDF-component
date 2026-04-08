#ifndef _TPS_H_
#define _TPS_H_

#include "esp_err.h"
#include <stdint.h>

#include "I2C.h"
#include "PCAL.h"

#define TPS_I2C_ADDR  0x48

#define TPS_PWR_GOOD  0b11111010
#define TPS_PWRUP_SEQ 0b11100100
#define TPS_PWRDN_SEQ 0b00011011

// IO expander pin wired to TPS65186 INT output
#define TPS_INT_PIN   IO_NUM_A6

class TPS
{
public:
  TPS(I2C &i2c);

  esp_err_t initSequences();
  esp_err_t enableRails();
  esp_err_t disableRails();
  esp_err_t setPowerUpSequence(uint8_t seq);
  esp_err_t setPowerDownSequence(uint8_t seq);
  uint8_t   readPowerGood();
  bool      waitPowerGood(bool target);

  // VCOM programming — call with eink power already on
  esp_err_t writeVCOM(double vcom, PCAL &expander);
  // Read VCOM back from TPS65186 registers
  double    readVCOM();

  // Temperature sensor — eink panel must be powered on
  int8_t    readTemperature();

private:
  esp_err_t writeReg(uint8_t reg, uint8_t val);
  uint8_t   readReg(uint8_t reg);

  i2c_master_dev_handle_t m_handle = NULL;
};

#endif
