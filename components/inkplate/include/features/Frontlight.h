#ifndef _FRONTLIGHT_H_
#define _FRONTLIGHT_H_

#include "esp_err.h"
#include <stdint.h>

#include "I2C.h"
#include "PCAL.h"

#define FRONTLIGHT_I2C_ADDR  0x5C >> 1

// IO expander pin wired to frontlight
#define FRONTLIGHT_EN_PIN    IO_NUM_B2

class Frontlight
{
public:
  Frontlight(I2C &i2c, PCAL &expander);

  esp_err_t setBrightness(uint8_t value);
  void      setState(bool enable);

private:
  PCAL                   *m_expander  = nullptr;
  i2c_master_dev_handle_t m_devHandle = NULL;
};

#endif
